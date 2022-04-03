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
#include <vector>

// WHY GREEDY MESHING IS GONE FOR GOOD:
//  First. It doesn't allow us to use "random"
// textures for the same voxel - I've already
// modified the original algorithm to support
// face-based comparison which made it even
// slower than before (if you were running a
// Win32 Debug build you should've felt that),
// adding random textures would be suicidal in
// terms of overall mesher performance.
//  Second. Voxel orientations - in future I want
// to have some voxels (in terms of gameplay those
// should be called blocks) to have different orientation
// than others, that's gonna be based on some kind of
// a value in some kind of an NBT-based taglist.
//  Third. Support for custom voxel models. Minecraft
// does that - you can define your own 3D-ish model
// for the bloxel and the mesher will cull the sides
// when a bloxel's (block/voxel) side is connected
// to an opaque bloxel nearby - I want to have that
// applied to STATIC_CUBE and STATIC_FLORA voxels. Also
// the support for custom models will reduce the amounts
// of pain I will have to go through to add slabs and stairs.

struct PackedVertex final {
    // [0]: 3x10-bit position vector and 1x2-bit shade.
    // [1]: 2x5-bit texture coordinates and 3x7-bit normal.
    // [2]: 1x32-bit texture atlas entry index.
    GLuint pack_0 {0};
    GLuint pack_1 {0};
    GLuint pack_2 {0};

    inline PackedVertex(const vector3f_t &position, const vector2f_t &texcoord, unsigned int shade, const vector3f_t &normal, GLuint atlas_entry_index)
        : pack_0(0), pack_1(0), pack_2(atlas_entry_index)
    {
        pack_0 |= (static_cast<GLuint>(position.x / 16.0f * 512.0f) & 0x3FF) << 22;
        pack_0 |= (static_cast<GLuint>(position.y / 16.0f * 512.0f) & 0x3FF) << 12;
        pack_0 |= (static_cast<GLuint>(position.z / 16.0f * 512.0f) & 0x3FF) << 2;
        pack_0 |= (static_cast<GLuint>(shade) & 0x03);
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

class WorkerContext final : mixin::NoCopy {
public:
    WorkerContext(const chunk_pos_t &cpos, thread_pool &pool)
        : cancelled(false), cpos(cpos), cache()
    {
        cacheChunk(cpos);
        cacheChunk(cpos + chunk_pos_t(0, 0, 1));
        cacheChunk(cpos - chunk_pos_t(0, 0, 1));
        cacheChunk(cpos + chunk_pos_t(0, 1, 0));
        cacheChunk(cpos - chunk_pos_t(0, 1, 0));
        cacheChunk(cpos + chunk_pos_t(1, 0, 0));
        cacheChunk(cpos - chunk_pos_t(1, 0, 0));
        future = pool.submit(std::bind(&WorkerContext::run, this));
    }

    void cancel()
    {
        cancelled = true;
    }

    bool isComplete() const
    {
        return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    bool isCancelled() const
    {
        return cancelled;
    }

    const chunk_pos_t &getChunkPos() const
    {
        return cpos;
    }

    const PackedMeshBuilder &getBuilder() const
    {
        return builder;
    }

    const size_t getMemoryUsage() const
    {
        return sizeof(WorkerContext) + sizeof(chunk_t) * cache.size();
    }

private:
    void run()
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

                            if(!checkTransparency(cpos, neighbour_lpos, voxel, neighbour_face))
                                continue;
                            pushFace(entry, lpos, def_face.first, base);
                        }
                    }
                }
            }
        }
    }

private:
    bool checkTransparency(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t compare_voxel, VoxelFace compare_face)
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

    void pushFace(const AtlasEntry *atlas_entry, const local_pos_t &lpos, VoxelFace face, PackedMeshIndex &base)
    {
        vector3f_t face_normal;
        local_pos_t offsets[4];
        vector2f_t texcoords[4];
        unsigned int shade;
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
                shade = 1;
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
                shade = 1;
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
                shade = 2;
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
                shade = 2;
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
                shade = 3;
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
                shade = 0;
                break;
            default:
                // oopsie
                return;
        }

        builder.vertex(PackedVertex(vector3f_t(lpos + offsets[0]), texcoords[0] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
        builder.vertex(PackedVertex(vector3f_t(lpos + offsets[1]), texcoords[1] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
        builder.vertex(PackedVertex(vector3f_t(lpos + offsets[2]), texcoords[2] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
        builder.vertex(PackedVertex(vector3f_t(lpos + offsets[3]), texcoords[3] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    void cacheChunk(const chunk_pos_t &cpos)
    {
        if(const ClientChunk *chunk = globals::chunks.find(cpos)) {
            // Each worker generates a mesh based on
            // a cached snapshot of the chunk and its
            // neighbouring chunks. It's memory inefficient
            // but a thread-safe solution.
            cache[cpos] = chunk->data;
        }
    }

private:
    bool cancelled;
    chunk_pos_t cpos;
    PackedMeshBuilder builder;
    std::unordered_map<chunk_pos_t, chunk_t> cache;
    std::shared_future<bool> future;
};

#if defined(NDEBUG)
constexpr static const size_t MAX_CHUNKS_PER_FRAME = 16;
constexpr static const size_t WORKER_POOL_SIZE = 4;
#else
// Debug builds for Win32 have a distinctive feature
// to completely and absolutely shit themselves wihen
// there's a lot of threading going on. I guess that's
// why Source tries to be as less threaded as possible.
constexpr static const size_t MAX_CHUNKS_PER_FRAME = 4;
constexpr static const size_t WORKER_POOL_SIZE = 1;
#endif

static thread_pool workers_pool(WORKER_POOL_SIZE);
static std::vector<WorkerContext *> workers;
static size_t memory_usage = 0;

void terrain_mesher::shutdown()
{
    for(WorkerContext *worker : workers)
        worker->cancel();
    workers_pool.wait_for_tasks();
    for(WorkerContext *worker : workers)
        delete worker;
    workers.clear();
    memory_usage = 0;
}

void terrain_mesher::update()
{
    size_t count = 0;
    auto worker = workers.begin();
    while(worker != workers.end()) {
        if((*worker)->isComplete()) {
            const PackedMeshBuilder &builder = (*worker)->getBuilder();

            if(const ClientChunk *chunk = globals::chunks.find((*worker)->getChunkPos())) {
                do {
                    StaticChunkMeshComponent &mesh = globals::registry.get_or_emplace<StaticChunkMeshComponent>(chunk->entity);

                    if(builder.empty()) {
                        mesh.cube = nullptr;
                        break;
                    }

                    if(!mesh.cube) {
                        mesh.cube = std::make_unique<Mesh>();
                        mesh.cube->ibo.create();
                        mesh.cube->vbo.create();
                        mesh.cube->vao.create();
                        mesh.cube->cmd.create();

                        mesh.cube->vao.setIndexBuffer(mesh.cube->ibo);
                        mesh.cube->vao.setVertexBuffer(0, mesh.cube->vbo, sizeof(PackedVertex));

                        mesh.cube->vao.enableAttribute(0, true);
                        mesh.cube->vao.enableAttribute(1, true);
                        mesh.cube->vao.enableAttribute(2, true);

                        mesh.cube->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_0), false);
                        mesh.cube->vao.setAttributeFormat(1, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_1), false);
                        mesh.cube->vao.setAttributeFormat(2, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_2), false);

                        mesh.cube->vao.setAttributeBinding(0, 0);
                        mesh.cube->vao.setAttributeBinding(1, 0);
                        mesh.cube->vao.setAttributeBinding(2, 0);
                    }

                    mesh.cube->ibo.resize(builder.isize(), builder.idata(), GL_STATIC_DRAW);
                    mesh.cube->vbo.resize(builder.vsize(), builder.vdata(), GL_STATIC_DRAW);
                    mesh.cube->cmd.set(GL_TRIANGLES, PACKED_MESH_INDEX, builder.icount(), 1, 0, 0, 0);

                    count++;
                } while(false);
            }

            memory_usage -= (*worker)->getMemoryUsage();

            delete (*worker);
            worker = workers.erase(worker);

            if(count >= MAX_CHUNKS_PER_FRAME)
                break;
            continue;
        }

        worker++;
    }

    const auto group = globals::registry.group<StaticChunkMeshNeedsUpdateComponent<VoxelType::STATIC_CUBE>>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : group.each()) {
        memory_usage += workers.emplace_back(new WorkerContext(chunk.cpos, workers_pool))->getMemoryUsage();
        globals::registry.remove<StaticChunkMeshNeedsUpdateComponent<VoxelType::STATIC_CUBE>>(entity);
    }
}

size_t terrain_mesher::getMemoryUsage()
{
    return memory_usage;
}
