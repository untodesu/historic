/*
 * chunk_mesher.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <deque>
#include <client/components/chunk_mesh.hpp>
#include <client/systems/chunk_mesher.hpp>
#include <client/globals.hpp>
#include <client/vertex.hpp>
#include <client/render/atlas.hpp>
#include <client/chunks.hpp>
#include <client/util/mesh_builder.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <thread_pool.hpp>
#include <shared/components/chunk.hpp>

using ChunkMeshBuilder = MeshBuilder<uint16_t, Vertex>;

class ThreadedMeshingContext final {
public:
    ThreadedMeshingContext(const chunkpos_t &cp, entt::entity owner);
    ~ThreadedMeshingContext();
    const bool hasVoxelAt(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel) const;
    const bool hasFaceAt(const chunkpos_t &cp, const localpos_t &lp, voxel_face_t face) const;
    void enqueue();
    void update();

    inline const bool isCancelled() const
    {
        return cancelled;
    }

    inline const bool isCompleted() const
    {
        return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    inline ChunkMeshBuilder &getBuilder()
    {
        return mesh_builder;
    }

    inline entt::entity getOwner() const
    {
        return owner;
    }

private:
    void threadFunc();

private:
    bool cancelled;
    chunkpos_t position;
    entt::entity owner;
    ChunkMeshBuilder mesh_builder;
    std::shared_future<bool> future;
};

using ThreadedMeshingContextPtr = std::shared_ptr<ThreadedMeshingContext>;
using ThreadedMeshingContextList = std::vector<ThreadedMeshingContextPtr>;

static thread_pool mesher_threads = thread_pool(4);
static ThreadedMeshingContextList mesher_workers;
static size_t mesher_workers_count = 0;
static bool cancel_meshing = false;

ThreadedMeshingContext::ThreadedMeshingContext(const chunkpos_t &cp, entt::entity owner)
    : cancelled(false), position(cp), owner(owner), mesh_builder()
{

}

ThreadedMeshingContext::~ThreadedMeshingContext()
{

}

void ThreadedMeshingContext::enqueue()
{
    future = mesher_threads.submit(std::bind(&ThreadedMeshingContext::threadFunc, this));
}

void ThreadedMeshingContext::update()
{
    cancelled = cancel_meshing || !globals::registry.valid(owner);
}

const bool ThreadedMeshingContext::hasVoxelAt(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel) const
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    if(const ClientChunk *cc = globals::chunks.find(toChunkPos(vp)))
        return cc->data.at(toVoxelIdx(toLocalPos(vp))) == voxel;
    return false;
}

const bool ThreadedMeshingContext::hasFaceAt(const chunkpos_t &cp, const localpos_t &lp, voxel_face_t face) const
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    if(const ClientChunk *cc = globals::chunks.find(toChunkPos(vp))) {
        if(const voxel_t voxel = cc->data.at(toVoxelIdx(toLocalPos(vp)))) {
            if(const VoxelDefEntry *vde = globals::voxels.find(voxel)) {
                const auto jt = vde->faces.find(face);
                if(jt != vde->faces.cend())
                    return !jt->second.transparent;
                return false;
            }

            spdlog::warn("Unkown voxel ID: {}", voxel);
            return false;
        }
    }

    return false;
}

static inline void pushQuad(ChunkMeshBuilder &builder, uint16_t &base, const Vertex data[4])
{
    for(int i = 0; i < 4; i++)
        builder.vertex(data[i]);
    builder.index(base + 0);
    builder.index(base + 1);
    builder.index(base + 2);
    builder.index(base + 2);
    builder.index(base + 3);
    builder.index(base + 0);
    base += 4;
}

static void greedyFace(ThreadedMeshingContext *ctx, const chunkpos_t &cp, const VoxelDefEntry &entry, const AtlasNode *node, voxel_t voxel, voxel_face_t face, uint16_t &base)
{
    ChunkMeshBuilder &builder = ctx->getBuilder();

    const voxel_face_t flip_face = flipVoxelFace(face);

    std::array<bool, CHUNK_AREA> mask;

    int16_t d = 0;
    switch(face) {
        case VOXEL_FACE_LF:
        case VOXEL_FACE_RT:
            d = 0;
            break;
        case VOXEL_FACE_UP:
        case VOXEL_FACE_DN:
            d = 1;
            break;
        case VOXEL_FACE_FT:
        case VOXEL_FACE_BK:
            d = 2;
            break;
        default:
            return;
    }

    const int16_t u = (d + 1) % 3;
    const int16_t v = (d + 2) % 3;
    localpos_t x = localpos_t(0, 0, 0);
    localpos_t q = localpos_t(0, 0, 0);
    q[d] = static_cast<int16_t>(voxelFaceNormal(flip_face));

    // Go through each slice of the chunk
    constexpr const int16_t CHUNK_SIZE_I16 = static_cast<int16_t>(CHUNK_SIZE);
    for(x[d] = 0; x[d] < CHUNK_SIZE_I16;) {
        size_t maskpos;

        // Generate the mask for this slice
        maskpos = 0;
        for(x[v] = 0; x[v] < CHUNK_SIZE_I16; x[v]++) {
            for(x[u] = 0; x[u] < CHUNK_SIZE_I16; x[u]++) {
                // We set the mask only if the current voxel is the one
                // and if the next present (by direction) has a solid face.
                mask[maskpos++] = ctx->hasVoxelAt(cp, x, voxel) && !ctx->hasFaceAt(cp, x + q, flip_face);
            }
        }

        x[d]++;

        // Produce quads
        maskpos = 0;
        for(int16_t j = 0; j < CHUNK_SIZE_I16; j++) {
            for(int16_t i = 0; i < CHUNK_SIZE_I16;) {
                if(mask[maskpos]) {
                    int16_t qw, qh;

                    // Calculate the quad width.
                    for(qw = 1; (i + qw) < CHUNK_SIZE_I16 && mask[maskpos + qw]; qw++);

                    // Calculate the quad height.
                    bool qh_done = false;
                    for(qh = 1; (j + qh) < CHUNK_SIZE_I16; qh++) {
                        for(int16_t k = 0; k < qw; k++) {
                            if(!mask[maskpos + k + qh * CHUNK_SIZE_I16]) {
                                qh_done = true;
                                break;
                            }
                        }

                        if(qh_done)
                            break;
                    }

                    x[u] = i;
                    x[v] = j;

                    float3 position = float3(x.x, x.y, x.z);
                    if(face & VOXEL_FACE_MASK_INV)
                        position[d] += q[d];

                    float2 texcoords[4];
                    float2 tc = float2(qw, qh) * node->max_uv;
                    switch(face) {
                        case VOXEL_FACE_LF:
                            texcoords[0] = float2(0.0f, 0.0f);
                            texcoords[1] = float2(tc.y, 0.0f);
                            texcoords[2] = float2(tc.y, tc.x);
                            texcoords[3] = float2(0.0f, tc.x);
                            break;
                        case VOXEL_FACE_RT:
                            texcoords[0] = float2(tc.y, 0.0f);
                            texcoords[1] = float2(tc.y, tc.x);
                            texcoords[2] = float2(0.0f, tc.x);
                            texcoords[3] = float2(0.0f, 0.0f);
                            break;
                        case VOXEL_FACE_FT:
                            texcoords[0] = float2(tc.x, 0.0f);
                            texcoords[1] = float2(tc.x, tc.y);
                            texcoords[2] = float2(0.0f, tc.y);
                            texcoords[3] = float2(0.0f, 0.0f);
                            break;
                        case VOXEL_FACE_BK:
                            texcoords[0] = float2(0.0f, 0.0f);
                            texcoords[1] = float2(tc.x, 0.0f);
                            texcoords[2] = float2(tc.x, tc.y);
                            texcoords[3] = float2(0.0f, tc.y);
                            break;
                        case VOXEL_FACE_UP:
                            texcoords[0] = float2(0.0f, tc.x);
                            texcoords[1] = float2(0.0f, 0.0f);
                            texcoords[2] = float2(tc.y, 0.0f);
                            texcoords[3] = float2(tc.y, tc.x);
                            break;
                        case VOXEL_FACE_DN:
                            texcoords[0] = float2(tc.y, tc.x);
                            texcoords[1] = float2(0.0f, tc.x);
                            texcoords[2] = float2(0.0f, 0.0f);
                            texcoords[3] = float2(tc.y, 0.0f);
                            break;
                    }

                    float3 du = FLOAT3_ZERO;
                    du[u] = static_cast<float>(qw);

                    float3 dv = FLOAT3_ZERO;
                    dv[v] = static_cast<float>(qh);

                    float3 normal = FLOAT3_ZERO;
                    normal[d] = voxelFaceNormal(face);

                    Vertex verts[4];
                    if(face & VOXEL_FACE_MASK_INV) {
                        verts[0] = Vertex { position, normal, texcoords[0], node->index };
                        verts[1] = Vertex { position + dv, normal, texcoords[1], node->index };
                        verts[2] = Vertex { position + du + dv, normal, texcoords[2], node->index };
                        verts[3] = Vertex { position + du, normal, texcoords[3], node->index };
                    }
                    else {
                        verts[0] = Vertex { position, normal, texcoords[0], node->index };
                        verts[1] = Vertex { position + du, normal, texcoords[1], node->index };
                        verts[2] = Vertex { position + du + dv, normal, texcoords[2], node->index };
                        verts[3] = Vertex { position + dv, normal, texcoords[3], node->index };
                    }

                    pushQuad(builder, base, verts);

                    // Cleanup
                    for(int16_t k = 0; k < qh; k++) {
                        for(int16_t l = 0; l < qw; l++) {
                            mask[maskpos + l + k * CHUNK_SIZE_I16] = false;
                        }
                    }

                    i += qw;
                    maskpos += qw;
                }
                else {
                    i++;
                    maskpos++;
                }
            }
        }
    }
}

void ThreadedMeshingContext::threadFunc()
{
    uint16_t base = 0;
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        if(it->second.type == VOXEL_SOLID) {
            for(const auto face : it->second.faces) {
                if(const AtlasNode *node = globals::solid_textures.getNode(face.second.texture)) {
                    greedyFace(this, position, it->second, node, it->first, face.first, base);
                    if(cancelled) {
                        mesh_builder.clear();
                        return;
                    }
                }
            }
        }
    }
}

static void pushWorker(ThreadedMeshingContextPtr ctx)
{
    for(auto it = mesher_workers.begin(); it != mesher_workers.end(); it++) {
        if(!(*it)) {
            *it = ctx;
            return;
        }
    }

    mesher_workers.push_back(ctx);
}

void chunk_mesher::shutdown()
{
    cancel_meshing = true;
    mesher_threads.wait_for_tasks();
}

void chunk_mesher::update()
{
    // New and possibly thread-safe routing for per-frame mesher:
    //  1.  Go through all the workers and update their status,
    //      this will update their cancelled status and mark them
    //      for deletion in the next loop.
    //  2.  Go through all the workers and generate meshes for completed
    //      but not cancelled ones, delete marked and completed workers.
    //  3.  Go through all the chunks marked for meshing and enqueue a
    //      new worker instance to the thread pool.

    // 1
    for(ThreadedMeshingContextPtr &it : mesher_workers) {
        if(!it)
            continue;
        it->update();
    }

    // 2
    for(auto it = mesher_workers.begin(); it != mesher_workers.end(); it++) {
        ThreadedMeshingContextPtr &ptr = *it;
        if(ptr && ptr->isCompleted()) {
            const entt::entity owner = ptr->getOwner();
            if(globals::registry.valid(owner)) {
                const ChunkMeshBuilder &builder = ptr->getBuilder();
                do {
                    if(builder.empty()) {
                        globals::registry.remove<ChunkMeshComponent>(owner);
                        break;
                    }

                    ChunkMeshComponent *mesh = globals::registry.try_get<ChunkMeshComponent>(owner);
                    if(!mesh) {
                        mesh = &globals::registry.emplace<ChunkMeshComponent>(owner);
                        mesh->ibo.create();
                        mesh->vbo.create();
                        mesh->vao.create();
                        mesh->cmd.create();
                        mesh->vao.setIndexBuffer(mesh->ibo);
                        mesh->vao.setVertexBuffer(0, mesh->vbo, sizeof(Vertex));

                        // UNDONE: for some unknown reason packed vertex
                        // was fucking up the UV coordinates so that it was
                        // shifted roughly one pixel down/left. For the time
                        // of active development there would be no packed vertices
                        // but if someone would insist on re-adding them I would be
                        // more than glad if packed vertices would be used instead :)

                        // Position
                        mesh->vao.enableAttribute(0, true);
                        mesh->vao.setAttributeFormat(0, GL_FLOAT, 3, offsetof(Vertex, position), false);
                        mesh->vao.setAttributeBinding(0, 0);

                        // Normal
                        mesh->vao.enableAttribute(1, true);
                        mesh->vao.setAttributeFormat(1, GL_FLOAT, 3, offsetof(Vertex, normal), false);
                        mesh->vao.setAttributeBinding(1, 0);

                        // Texcoord
                        mesh->vao.enableAttribute(2, true);
                        mesh->vao.setAttributeFormat(2, GL_FLOAT, 2, offsetof(Vertex, texcoord), false);
                        mesh->vao.setAttributeBinding(2, 0);

                        // Atlas ID
                        mesh->vao.enableAttribute(3, true);
                        mesh->vao.setAttributeFormat(3, GL_UNSIGNED_INT, 1, offsetof(Vertex, atlas_id), false);
                        mesh->vao.setAttributeBinding(3, 0);
                    }

                    mesh->ibo.resize(builder.isize(), builder.idata(), GL_STATIC_DRAW);
                    mesh->vbo.resize(builder.vsize(), builder.vdata(), GL_STATIC_DRAW);
                    mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, builder.icount(), 1, 0, 0, 0);
                } while(false);
            }

            ptr = nullptr;
            mesher_workers_count--;
        }
    }

    const auto group = globals::registry.group<ChunkFlaggedForMeshingComponent>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : group.each()) {
        globals::registry.remove<ChunkFlaggedForMeshingComponent>(entity);
        if(const ClientChunk *cc = globals::chunks.find(chunk.position)) {
            ThreadedMeshingContextPtr ctx = std::make_shared<ThreadedMeshingContext>(chunk.position, entity);
            pushWorker(ctx);
            ctx->enqueue();
            mesher_workers_count++;
        }
    }
}

void chunk_mesher::stats(ChunkMesherStats &s)
{
    s.thread_queue_size = mesher_threads.get_tasks_queued();
    s.worker_quota = mesher_workers.size();
    s.worker_count = mesher_workers_count;
}

#if 0
using ChunkMeshBuilder = MeshBuilder<uint16_t, Vertex>;

struct ChunkIsMeshingComponent final {};

class ThreadedMeshWorker final {
public:
    ThreadedMeshWorker(const chunkpos_t &cp, entt::entity owner)
        : builder(), cancelled(false), done(false), cp(cp), owner(owner)
    {
        trySetChunk(cp);
        trySetChunk(cp + chunkpos_t(0, 0, 1));
        trySetChunk(cp - chunkpos_t(0, 0, 1));
        trySetChunk(cp + chunkpos_t(0, 1, 0));
        trySetChunk(cp - chunkpos_t(0, 1, 0));
        trySetChunk(cp + chunkpos_t(1, 0, 0));
        trySetChunk(cp - chunkpos_t(1, 0, 0));
    }

    void thread()
    {
        threadStub();
        spdlog::info("THRD: done meshing {} {} {}", cp.x, cp.y, cp.z);
        done = true;
    }

    void update()
    {
        // Called in main thread.
        // Cancels the job if the entity got destroyed
        // or marked for meshing once again.
        if(globals::registry.valid(owner) && !globals::registry.all_of<ChunkFlaggedForMeshingComponent>(owner))
            return;
        cancelled = true;
    }

private:
    inline void threadStub()
    {
        uint16_t base = 0;
        for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
            if(it->second.type == VOXEL_SOLID) {
                for(const auto face : it->second.faces) {
                    if(const AtlasNode *node = globals::solid_textures.getNode(face.second.texture)) {
                        greedyFace(it->second, node, it->first, face.first, base);
                        if(cancelled)
                            return;
                    }
                }
            }
        }
    }



private:
    inline void pushQuad(uint16_t &base, const Vertex data[4])
    {
        for(int i = 0; i < 4; i++)
            builder.vertex(data[i]);
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    inline const bool hasVoxel(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel)
    {
        const voxelpos_t vp = toVoxelPos(cp, lp);
        const auto it = data.find(toChunkPos(vp));
        if(it != data.cend())
            return it->second.data.at(toVoxelIdx(toLocalPos(vp))) == voxel;
        return false;
    }

    inline const bool hasFace(const chunkpos_t &cp, const localpos_t &lp, voxel_face_t face)
    {
        const voxelpos_t vp = toVoxelPos(cp, lp);
        const auto it = data.find(toChunkPos(vp));
        if(it != data.cend()) {
            if(voxel_t voxel = it->second.data.at(toVoxelIdx(toLocalPos(vp)))) {
                if(const VoxelDefEntry *entry = globals::voxels.find(voxel)) {
                    const auto fit = entry->faces.find(face);
                    if(fit != entry->faces.cend())
                        return !fit->second.transparent;
                    return false;
                }

                return false;
            }
        }

        return false;
    }

    size_t trySetChunk(const chunkpos_t &pos)
    {
        if(ClientChunk *cc = globals::chunks.find(pos)) {
            data[pos] = *cc;
            return CHUNK_VOLUME * sizeof(voxel_t);
        }
        
        return 0;
    }

public:
    chunkpos_t cp;
    entt::entity owner;
    std::atomic<bool> cancelled, done;
    ChunkMeshBuilder builder;
    std::unordered_map<chunkpos_t, ClientChunk> data;
};

template<>
struct std::hash<ThreadedMeshWorker> final {
    inline const size_t operator()(const ThreadedMeshWorker &worker) const
    {
        return std::hash<chunkpos_t>()(worker.cp);
    }
};

using ThreadedMeshWorkerList = std::vector<std::shared_ptr<ThreadedMeshWorker>>;

thread_pool mesher_threads(4);
ThreadedMeshWorkerList mesher_workers;
static bool cancel_meshing = false;

void chunk_mesher::shutdown()
{
    cancel_meshing = true;
    mesher_threads.wait_for_tasks();
}

void chunk_mesher::update()
{
    // THINGS TO DO HERE:
    // X 1. UPDATE (CHECK) THREADED WORKERS
    // X 2. CLEANUP CANCELLED AND COMPLETED WORKERS
    // X 2.1. Go through workers list and find cancelled and completed workers
    // X 2.2. Generate meshes for those workers
    // X 2.3. Remove the worker...
    // X 3. QUEUE NEW WORKERS <- FIXME: DO NOT ENQUEUE THINGS ALREADY IN-PROGRESS
    // This ensures we don't have duplicate workers

    for(std::shared_ptr<ThreadedMeshWorker> &worker : mesher_workers) {
        // update() ensures we don't continue if the chunk
        // has been destroyed or marked for meshing
        worker->update();
        worker->cancelled = cancel_meshing || worker->cancelled;
    }

    for(ThreadedMeshWorkerList::const_iterator it = mesher_workers.cbegin(); it != mesher_workers.cend();) {
        std::shared_ptr<ThreadedMeshWorker> worker = *it;
        globals::registry.remove<ChunkIsMeshingComponent>(worker->owner);
        if(worker->done) {
            spdlog::info("MAIN: done meshing {} {} {}", worker->cp.x, worker->cp.y, worker->cp.z);
            if(!worker->cancelled && !worker->builder.empty() && globals::registry.valid(worker->owner)) {
                ChunkMeshComponent *mesh = globals::registry.try_get<ChunkMeshComponent>(worker->owner);
                if(!mesh) {
                    mesh = &globals::registry.emplace<ChunkMeshComponent>(worker->owner);
                    mesh->ibo.create();
                    mesh->vbo.create();
                    mesh->vao.create();
                    mesh->cmd.create();
                    mesh->vao.setIndexBuffer(mesh->ibo);
                    mesh->vao.setVertexBuffer(0, mesh->vbo, sizeof(Vertex));

                    // UNDONE: for some unknown reason packed vertex
                    // was fucking up the UV coordinates so that it was
                    // shifted roughly one pixel down/left. For the time
                    // of active development there would be no packed vertices
                    // but if someone would insist on re-adding them I would be
                    // more than glad if packed vertices would be used instead :)

                    // Position
                    mesh->vao.enableAttribute(0, true);
                    mesh->vao.setAttributeFormat(0, GL_FLOAT, 3, offsetof(Vertex, position), false);
                    mesh->vao.setAttributeBinding(0, 0);

                    // Normal
                    mesh->vao.enableAttribute(1, true);
                    mesh->vao.setAttributeFormat(1, GL_FLOAT, 3, offsetof(Vertex, normal), false);
                    mesh->vao.setAttributeBinding(1, 0);

                    // Texcoord
                    mesh->vao.enableAttribute(2, true);
                    mesh->vao.setAttributeFormat(2, GL_FLOAT, 2, offsetof(Vertex, texcoord), false);
                    mesh->vao.setAttributeBinding(2, 0);

                    // Atlas ID
                    mesh->vao.enableAttribute(3, true);
                    mesh->vao.setAttributeFormat(3, GL_UNSIGNED_INT, 1, offsetof(Vertex, atlas_id), false);
                    mesh->vao.setAttributeBinding(3, 0);
                }

                mesh->ibo.resize(worker->builder.isize(), worker->builder.idata(), GL_STATIC_DRAW);
                mesh->vbo.resize(worker->builder.vsize(), worker->builder.vdata(), GL_STATIC_DRAW);
                mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, worker->builder.icount(), 1, 0, 0, 0);
            }

            it = mesher_workers.erase(it);
            continue;
        }

        it++;
    }

    const auto group = globals::registry.group<ChunkFlaggedForMeshingComponent>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : group.each()) {
        globals::registry.remove<ChunkFlaggedForMeshingComponent>(entity);
        if(!globals::registry.all_of<ChunkIsMeshingComponent>(entity)) {
            if(ClientChunk *cc = globals::chunks.find(chunk.position)) {
                std::shared_ptr<ThreadedMeshWorker> worker = std::make_shared<ThreadedMeshWorker>(chunk.position, entity);
                mesher_workers.push_back(worker);
                mesher_threads.push_task(std::bind(&ThreadedMeshWorker::thread, worker));
            }
        }
    }
}
#endif
