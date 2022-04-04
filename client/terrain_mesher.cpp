/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/comp/static_chunk_mesh_component.hpp>
#include <client/comp/static_chunk_mesh_needs_update_component.hpp>
#include <client/globals.hpp>
#include <client/mesh_builder.hpp>
#include <client/terrain_mesher.hpp>
#include <common/comp/chunk_component.hpp>
#include <common/math/constexpr.hpp>
#include <common/math/crc64.hpp>
#include <common/mixin.hpp>
#include <common/voxels.hpp>
#include <spdlog/spdlog.h>
#include <thread_pool.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct PackedVertex final {
    // [0]: 3x10-bit position vector.
    // [1]: 2x5-bit texture coordinates and 3x7-bit normal.
    // [2]: 1x32-bit texture atlas entry index.
    GLuint pack_0 {0};
    GLuint pack_1 {0};
    GLuint pack_2 {0};

    inline PackedVertex(const vector3f_t &position, const vector2f_t &texcoord, const vector3f_t &normal, GLuint atlas_entry_index)
        : pack_0(0), pack_1(0), pack_2(atlas_entry_index)
    {
        pack_0 |= (static_cast<GLuint>(position.x / 16.0f * 512.0f) & 0x3FF) << 22;
        pack_0 |= (static_cast<GLuint>(position.y / 16.0f * 512.0f) & 0x3FF) << 12;
        pack_0 |= (static_cast<GLuint>(position.z / 16.0f * 512.0f) & 0x3FF) << 2;
        pack_1 |= (static_cast<GLuint>(math::max(texcoord.x, 0.0f) * 16.0f) & 0x1F) << 27;
        pack_1 |= (static_cast<GLuint>(math::max(texcoord.y, 0.0f) * 16.0f) & 0x1F) << 22;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.x, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F) << 14;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.y, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F) << 7;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.z, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F);
    }
};

using PackedMeshIndex = GLushort;
using PackedMeshBuilder = MeshBuilder<PackedMeshIndex, PackedVertex>;
constexpr static const GLenum PACKED_MESH_INDEX = GL_UNSIGNED_SHORT;

static void makePackedMesh(std::unique_ptr<Mesh> &mesh)
{
    if(!mesh) {
        mesh.reset(new Mesh());
        mesh->ibo.create();
        mesh->vbo.create();
        mesh->vao.create();
        mesh->cmd.create();

        mesh->vao.setIndexBuffer(mesh->ibo);
        mesh->vao.setVertexBuffer(0, mesh->vbo, sizeof(PackedVertex));

        mesh->vao.enableAttribute(0, true);
        mesh->vao.enableAttribute(1, true);
        mesh->vao.enableAttribute(2, true);

        mesh->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_0), false);
        mesh->vao.setAttributeFormat(1, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_1), false);
        mesh->vao.setAttributeFormat(2, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_2), false);

        mesh->vao.setAttributeBinding(0, 0);
        mesh->vao.setAttributeBinding(1, 0);
        mesh->vao.setAttributeBinding(2, 0);
    }
}

class IWorkerContext : public mixin::NoCopy {
public:
    IWorkerContext(const chunk_pos_t &cpos, thread_pool &pool);
    virtual ~IWorkerContext() = default;

    void cancelTask();
    bool isTaskCancelled() const;
    bool isTaskComplete() const;

    const chunk_pos_t &getChunkPos() const;
    const PackedMeshBuilder &getMeshBuilder() const;

    virtual void run() = 0;
    virtual void finalize(StaticChunkMeshComponent &mesh) = 0;

protected:
    bool isTransparent(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t compare_voxel, VoxelFace compare_face);
    void pushFace(const AtlasEntry *atlas_entry, const local_pos_t &lpos, VoxelFace face, PackedMeshIndex &base);

private:
    void cacheChunk(const chunk_pos_t &cpos);
    void runThread();

protected:
    bool cancelled;
    chunk_pos_t cpos;
    PackedMeshBuilder builder;
    std::unordered_map<chunk_pos_t, chunk_t> cache;

private:
    std::shared_future<bool> future;
};

IWorkerContext::IWorkerContext(const chunk_pos_t &cpos, thread_pool &pool)
    : cancelled(false), cpos(cpos), builder(), cache(), future()
{
    // Pre-cache chunk data
    cacheChunk(cpos);
    cacheChunk(cpos + chunk_pos_t(0, 0, 1));
    cacheChunk(cpos - chunk_pos_t(0, 0, 1));
    cacheChunk(cpos + chunk_pos_t(0, 1, 0));
    cacheChunk(cpos - chunk_pos_t(0, 1, 0));
    cacheChunk(cpos + chunk_pos_t(1, 0, 0));
    cacheChunk(cpos - chunk_pos_t(1, 0, 0));

    // Queue ourselves for the task
    future = pool.submit(std::bind(&IWorkerContext::runThread, this));
}

void IWorkerContext::cancelTask()
{
    // This flag should tell the implementation
    // that something wants it to drop its work.
    cancelled = true;
}

bool IWorkerContext::isTaskCancelled() const
{
    return cancelled;
}

bool IWorkerContext::isTaskComplete() const
{
    return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

const chunk_pos_t &IWorkerContext::getChunkPos() const
{
    return cpos;
}

const PackedMeshBuilder &IWorkerContext::getMeshBuilder() const
{
    return builder;
}

bool IWorkerContext::isTransparent(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t compare_voxel, VoxelFace compare_face)
{
    const voxel_pos_t vpos = world::getVoxelPosition(cpos, lpos);
    const auto chunk = cache.find(world::getChunkPosition(vpos));
    if(chunk != cache.cend()) {
        const voxel_t voxel = chunk->second.at(world::getVoxelIndex(world::getLocalPosition(vpos)));
        if(compare_voxel != voxel) {
            if(const VoxelDefEntry *def = globals::voxels.find(voxel)) {
                const auto face = def->faces.find(compare_face);
                if(face != def->faces.cend())
                    return face->second.transparent;
                return true;
            }

            return true;
        }

        // Assuming contiguous voxels
        // with the same type always have
        // no faces inbetween each other.
        return false;
    }

    return true;
}

void IWorkerContext::pushFace(const AtlasEntry *atlas_entry, const local_pos_t &lpos, VoxelFace face, PackedMeshIndex &base)
{
    vector3f_t face_normal;
    local_pos_t offsets[4];
    vector2f_t texcoords[4];
    switch(face) {
        case VoxelFace::LF:
            face_normal = vector3f_t(-1.0f, 0.0f, 0.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(0, 0, 1);
            offsets[2] = local_pos_t(0, 1, 1);
            offsets[3] = local_pos_t(0, 1, 0);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            break;
        case VoxelFace::RT:
            face_normal = vector3f_t(1.0f, 0.0f, 0.0f);
            offsets[0] = local_pos_t(1, 0, 0);
            offsets[1] = local_pos_t(1, 1, 0);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(1, 0, 1);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            break;
        case VoxelFace::FT:
            face_normal = vector3f_t(0.0f, 0.0f, 1.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(0, 1, 0);
            offsets[2] = local_pos_t(1, 1, 0);
            offsets[3] = local_pos_t(1, 0, 0);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            break;
        case VoxelFace::BK:
            face_normal = vector3f_t(0.0f, 0.0f, -1.0f);
            offsets[0] = local_pos_t(0, 0, 1);
            offsets[1] = local_pos_t(1, 0, 1);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(0, 1, 1);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            break;
        case VoxelFace::UP:
            face_normal = vector3f_t(0.0f, 1.0f, 0.0f);
            offsets[0] = local_pos_t(0, 1, 0);
            offsets[1] = local_pos_t(0, 1, 1);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(1, 1, 0);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            break;
        case VoxelFace::DN:
            face_normal = vector3f_t(0.0f, -1.0f, 0.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(1, 0, 0);
            offsets[2] = local_pos_t(1, 0, 1);
            offsets[3] = local_pos_t(0, 0, 1);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            break;
        default:
            // oopsie
            return;
    }

    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[0]), texcoords[0] * atlas_entry->max_texcoord, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[1]), texcoords[1] * atlas_entry->max_texcoord, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[2]), texcoords[2] * atlas_entry->max_texcoord, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[3]), texcoords[3] * atlas_entry->max_texcoord, face_normal, atlas_entry->entry_index));
    builder.index(base + 0);
    builder.index(base + 1);
    builder.index(base + 2);
    builder.index(base + 2);
    builder.index(base + 3);
    builder.index(base + 0);
    base += 4;
}

void IWorkerContext::cacheChunk(const chunk_pos_t &cpos)
{
    if(const ClientChunk *chunk = globals::chunks.find(cpos)) {
        // Each worker generates a mesh based on
        // a cached snapshot of the chunk and its
        // neighbouring chunks. It's memory inefficient
        // but a thread-safe solution.
        cache[cpos] = chunk->data;
    }
}

void IWorkerContext::runThread()
{
    run();
}

template<VoxelType voxel_type>
class WorkerContext final : public IWorkerContext {
public:
    WorkerContext(const chunk_pos_t &cpos, thread_pool &pool);
    void run() override;
    void finalize(StaticChunkMeshComponent &mesh) override;
};

template<VoxelType voxel_type>
WorkerContext<voxel_type>::WorkerContext(const chunk_pos_t &cpos, thread_pool &pool)
    : IWorkerContext(cpos, pool)
{

}

template<>
void WorkerContext<VoxelType::STATIC_CUBE>::run()
{
    PackedMeshIndex base = 0;
    const auto chunk = cache.find(cpos);
    if(chunk != cache.cend()) {
        for(voxel_idx_t i = 0; i < CHUNK_VOLUME; i++) {
            const voxel_t voxel = chunk->second.at(i);
            if(const VoxelDefEntry *def = globals::voxels.find(voxel)) {
                const local_pos_t lpos = world::getLocalPosition(i);
                const voxel_pos_t vpos = world::getVoxelPosition(cpos, lpos);

                // Choosing a random texture - we just hash the bastard.
                const int64_t vpos_sum = vpos.x + vpos.y + vpos.z;
                const hash_t vpos_hash = math::crc64(&vpos_sum, sizeof(vpos_sum));

                for(const auto &def_face : def->faces) {
                    if(const AtlasEntry *entry = globals::terrain_atlas.find(def_face.second.textures[vpos_hash % def_face.second.textures.size()])) {
                        VoxelFace neighbour_face = VoxelFace::LF;
                        local_pos_t neighbour_lpos = { 0, 0, 0 };
                        switch(def_face.first) {
                            case VoxelFace::LF:
                                neighbour_face = VoxelFace::RT;
                                neighbour_lpos = lpos - local_pos_t(1, 0, 0);
                                break;
                            case VoxelFace::RT:
                                neighbour_face = VoxelFace::LF;
                                neighbour_lpos = lpos + local_pos_t(1, 0, 0);
                                break;
                            case VoxelFace::FT:
                                neighbour_face = VoxelFace::BK;
                                neighbour_lpos = lpos - local_pos_t(0, 0, 1);
                                break;
                            case VoxelFace::BK:
                                neighbour_face = VoxelFace::FT;
                                neighbour_lpos = lpos + local_pos_t(0, 0, 1);
                                break;
                            case VoxelFace::UP:
                                neighbour_face = VoxelFace::DN;
                                neighbour_lpos = lpos + local_pos_t(0, 1, 0);
                                break;
                            case VoxelFace::DN:
                                neighbour_face = VoxelFace::UP;
                                neighbour_lpos = lpos - local_pos_t(0, 1, 0);
                                break;
                        }

                        if(!isTransparent(cpos, neighbour_lpos, voxel, neighbour_face))
                            continue;
                        pushFace(entry, lpos, def_face.first, base);
                    }
                }
            }
        }
    }
}

template<>
void WorkerContext<VoxelType::STATIC_CUBE>::finalize(StaticChunkMeshComponent &mesh)
{
    if(builder.empty()) {
        mesh.cube = nullptr;
        return;
    }

    // make sure we are set up
    makePackedMesh(mesh.cube);

    mesh.cube->ibo.resize(builder.isize(), builder.idata(), GL_STATIC_DRAW);
    mesh.cube->vbo.resize(builder.vsize(), builder.vdata(), GL_STATIC_DRAW);
    mesh.cube->cmd.set(GL_TRIANGLES, PACKED_MESH_INDEX, builder.isize(), 1, 0, 0, 0);
}

#if defined(NDEBUG)
constexpr static const size_t MAX_WORKERS_PER_FRAME = 16;
constexpr static const size_t WORKER_POOL_SIZE = 4;
#else
// Debug builds for Win32 have a distinctive feature
// to completely and absolutely shit themselves wihen
// there's a lot of threading going on. I guess that's
// why Source tries to be as less threaded as possible.
constexpr static const size_t MAX_WORKERS_PER_FRAME = 1;
constexpr static const size_t WORKER_POOL_SIZE = 1;
#endif

static thread_pool workers_pool(WORKER_POOL_SIZE);
static std::vector<IWorkerContext *> workers;
static size_t memory_usage = 0;

void terrain_mesher::shutdown()
{
    for(IWorkerContext *worker : workers)
        worker->cancelTask();
    workers_pool.wait_for_tasks();
    for(IWorkerContext *worker : workers)
        delete worker;
    workers.clear();
    memory_usage = 0;
}

template<VoxelType voxel_type>
static void queueNewWorkers(size_t &worker_count)
{
    const auto group = globals::registry.group<StaticChunkMeshNeedsUpdateComponent<voxel_type>>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : group.each()) {
        if(worker_count >= MAX_WORKERS_PER_FRAME)
            break;
        workers.push_back(new WorkerContext<voxel_type>(chunk.cpos, workers_pool));
        globals::registry.remove<StaticChunkMeshNeedsUpdateComponent<voxel_type>>(entity);
        worker_count++;
    }
}

void terrain_mesher::update()
{
    size_t worker_count;

    worker_count = 0;
    auto worker_it = workers.begin();
    while(worker_it != workers.end()) {
        IWorkerContext *worker = *worker_it;
        if(worker->isTaskComplete()) {
            const ClientChunk *chunk = globals::chunks.find(worker->getChunkPos());
            if(chunk) {
                // This method should create a mesh object
                // if possible and fill it up with vertices.
                // If no data was generated, removes the mesh object.
                worker->finalize(globals::registry.get_or_emplace<StaticChunkMeshComponent>(chunk->entity));
                worker_count++;
            }
            
            delete worker;
            worker_it = workers.erase(worker_it);

            if(worker_count >= MAX_WORKERS_PER_FRAME)
                break;
            continue;
        }

        worker++;
    }

    worker_count = 0;
    queueNewWorkers<VoxelType::STATIC_CUBE>(worker_count);
}
