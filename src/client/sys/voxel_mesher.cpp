/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/globals.hpp>
#include <client/vertex.hpp>
#include <client/util/mesh_builder.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <thread_pool.hpp>

struct MesherData final {
    using map_type = std::unordered_map<chunkpos_t, voxel_array_t>;
    chunkpos_t self_pos;
    map_type::const_iterator self_data;
    map_type data;

    void trySetChunk(const chunkpos_t &cp)
    {
        voxel_array_t *chunk = globals::chunks.find(cp);
        if(!chunk)
            return;
        data[cp] = *chunk;
    }
};

using ChunkMeshBuilder = MeshBuilder<uint16_t, VoxelVertex>;

static void pushQuad(ChunkMeshBuilder *builder, uint16_t &base, const VoxelVertex data[4])
{
    for(int i = 0; i < 4; i++)
        builder->vertex(data[i]);
    builder->index(base + 0);
    builder->index(base + 1);
    builder->index(base + 2);
    builder->index(base + 2);
    builder->index(base + 3);
    builder->index(base + 0);
    base += 4;
}

// A lot of copypasta. Too bad!
static void pushFace(ChunkMeshBuilder *builder, const AtlasNode *node, const localpos_t &lp, voxel_face_t face, uint16_t &base)
{
    const float3_t lpf = float3_t(lp);

    if(face & VOXEL_FACE_LF) {
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_RT) {
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_BK) {
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_FT) {
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_UP) {
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_DN) {
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder->vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder->index(base + 0);
        builder->index(base + 1);
        builder->index(base + 2);
        builder->index(base + 2);
        builder->index(base + 3);
        builder->index(base + 0);
        base += 4;
    }
}

static bool isOccupied(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_face_t back)
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    if(const voxel_array_t *chunk = globals::chunks.find(toChunkPos(vp))) {
        const voxel_t compare = chunk->at(toVoxelIdx(toLocalPos(vp)));
        if(compare != voxel) {
            const VoxelInfo *info = globals::voxels.tryGet(compare);
            return info ? !(info->transparency & back) : false;
        }

        // Consecutive voxels __always__ do that
        return true;
    }

    return false;
}

// HACK: when shutting down we need
// to cancel all the meshing tasks
static bool cancel_meshing = false;
static thread_pool mesher_pool(8);

static void genMesh(ChunkMeshBuilder *builder, const chunkpos_t &cp)
{
    uint16_t base = 0;
    if(const voxel_array_t *chunk = globals::chunks.find(cp)) {
        for(voxelidx_t i = 0; i < CHUNK_VOLUME; i++) {
            const voxel_t voxel = chunk->at(i);
            const VoxelInfo *info = globals::voxels.tryGet(voxel);
            if(info && info->type == VoxelType::SOLID) {
                for(const VoxelFaceInfo &face : info->faces) {
                    if(const AtlasNode *node = globals::solid_textures.getNode(face.texture)) {
                        const localpos_t lp = toLocalPos(i);
                        voxel_face_t mask = face.mask;
                        if((mask & VOXEL_FACE_LF) && isOccupied(cp, lp - localpos_t(1, 0, 0), voxel, VOXEL_FACE_RT))
                            mask &= ~VOXEL_FACE_LF;
                        if((mask & VOXEL_FACE_RT) && isOccupied(cp, lp + localpos_t(1, 0, 0), voxel, VOXEL_FACE_LF))
                            mask &= ~VOXEL_FACE_RT;
                        if((mask & VOXEL_FACE_BK) && isOccupied(cp, lp + localpos_t(0, 0, 1), voxel, VOXEL_FACE_FT))
                            mask &= ~VOXEL_FACE_BK;
                        if((mask & VOXEL_FACE_FT) && isOccupied(cp, lp - localpos_t(0, 0, 1), voxel, VOXEL_FACE_BK))
                            mask &= ~VOXEL_FACE_FT;
                        if((mask & VOXEL_FACE_UP) && isOccupied(cp, lp + localpos_t(0, 1, 0), voxel, VOXEL_FACE_DN))
                            mask &= ~VOXEL_FACE_UP;
                        if((mask & VOXEL_FACE_DN) && isOccupied(cp, lp - localpos_t(0, 1, 0), voxel, VOXEL_FACE_UP))
                            mask &= ~VOXEL_FACE_DN;
                        pushFace(builder, node, lp, mask, base);
                    }
                }
            }

            if(cancel_meshing) {
                builder->clear();
                return;
            }
        }
    }
}

struct ThreadedVoxelMesherComponent final {
    ChunkMeshBuilder *builder { nullptr };
    std::shared_future<bool> future;
};


void voxel_mesher::shutdown()
{
    cancel_meshing = true;
    mesher_pool.wait_for_tasks();
}

// UNDONE: better greedy meshing?
void voxel_mesher::update()
{
    // Firstly we go through things that require meshing.
    const auto pending_group = globals::registry.group<NeedsVoxelMeshComponent>(entt::get<chunkpos_t>);
    for(const auto [entity, chunkpos] : pending_group.each()) {
        globals::registry.remove<VoxelMeshComponent>(entity);
        globals::registry.remove<NeedsVoxelMeshComponent>(entity);

        ThreadedVoxelMesherComponent &mesher = globals::registry.emplace_or_replace<ThreadedVoxelMesherComponent>(entity);
        mesher.builder = new ChunkMeshBuilder();
        mesher.future = mesher_pool.submit(genMesh, mesher.builder, chunkpos);
    }

    // Secondly we go through finished tasks
    const auto finished_view = globals::registry.view<ThreadedVoxelMesherComponent>();
    for(const auto [entity, mesher] : finished_view.each()) {
        if(mesher.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(!mesher.builder->empty()) {                
                VoxelMeshComponent &mesh = globals::registry.emplace_or_replace<VoxelMeshComponent>(entity);
                mesh.ibo.create();
                mesh.vbo.create();
                mesh.vao.create();
                mesh.cmd.create();
                mesh.ibo.storage(mesher.builder->isize(), mesher.builder->idata(), 0);
                mesh.vbo.storage(mesher.builder->vsize(), mesher.builder->vdata(), 0);
                mesh.vao.setIndexBuffer(mesh.ibo);
                mesh.vao.setVertexBuffer(0, mesh.vbo, sizeof(VoxelVertex));
                mesh.vao.enableAttribute(0, true);
                mesh.vao.setAttributeFormat(0, GL_UNSIGNED_INT, 2, offsetof(VoxelVertex, pack), false);
                mesh.vao.setAttributeBinding(0, 0);
                mesh.cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, mesher.builder->icount(), 1, 0, 0, 0);
            }

            delete mesher.builder;
            globals::registry.remove<ThreadedVoxelMesherComponent>(entity);
        }
    }
}
