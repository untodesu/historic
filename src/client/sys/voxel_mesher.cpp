/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/globals.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/res.hpp>
#include <uvre/uvre.hpp>
#include <client/vertex.hpp>
#include <client/util/mesh_builder.hpp>
#include <shared/voxels.hpp>
#include <shared/res.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <client/util/voxutils.hpp>

template<>
struct std::hash<chunkpos_t> final {
    const size_t operator()(const chunkpos_t &cp) const
    {
        // This at least would give some difference.
        return toVoxelIdx(localpos_t(cp.x, cp.y, cp.z));
    }
};

struct MesherData final {
    using map_type = std::unordered_map<chunkpos_t, voxel_array_t>;
    chunkpos_t self_pos;
    map_type::const_iterator self_data;
    map_type data;

    void trySetChunk(const chunkpos_t &cp)
    {
        voxel_array_t voxels;
        if(!voxutils::chunk(cp, voxels))
            return;
        data[cp] = voxels;
    }
};

using ChunkMeshBuilder = MeshBuilder<uvre::Index16, VoxelVertex>;

// A lot of copypasta. Too bad!
static void pushFace(ChunkMeshBuilder &builder, const AtlasNode *&node, const localpos_t &lp, voxel_face_t face, uvre::Index16 &base)
{
    const float3_t lpf = float3_t(lp);
    uvre::Index32 num_faces = 0;

    if(face & VOXEL_FACE_LF) {
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_RT) {
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_BK) {
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_FT) {
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_UP) {
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_DN) {
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 1.0f) * node->max_uv, node->index));
        builder.vertex(VoxelVertex(lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 1.0f) * node->max_uv, node->index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
    }
}

static bool isOccupied(const MesherData &data, const localpos_t &lp, voxel_t voxel, voxel_face_t back)
{
    const voxelpos_t vp = toVoxelPos(data.self_pos, lp);
    const chunkpos_t cp = toChunkPos(vp);
    const localpos_t lp_patch = toLocalPos(vp);
    const auto &it = data.data.find(cp);
    if(it != data.data.cend()) {
        const voxel_t compare = it->second[toVoxelIdx(lp_patch)];
        if(compare != voxel) {
            const VoxelInfo *info = globals::voxels.tryGet(compare);
            return info ? !(info->transparency & back) : false;
        }

        // Consecutive voxels _always_ do that
        return true;
    }

    return false;
}

static void genMesh(ChunkMeshBuilder &builder, const MesherData &data)
{
    builder.clear();
    uvre::Index16 base = 0;
    for(voxelidx_t i = 0; i < CHUNK_VOLUME; i++) {
        const voxel_t voxel = data.self_data->second[i];
        const VoxelInfo *info = globals::voxels.tryGet(voxel);
        if(info && info->type == VoxelType::SOLID) {
            for(const VoxelFaceInfo &face : info->faces) {
                // UNDONE: we should push() that only when we finish
                // registering voxel information in VoxelDef.
                const AtlasNode *node = globals::solid_textures.getNode(face.texture);
                if(node) {
                    const localpos_t lp = toLocalPos(i);
                    voxel_face_t mask = face.mask;
                    if((mask & VOXEL_FACE_LF) && isOccupied(data, lp - localpos_t(1, 0, 0), voxel, VOXEL_FACE_RT))
                        mask &= ~VOXEL_FACE_LF;
                    if((mask & VOXEL_FACE_RT) && isOccupied(data, lp + localpos_t(1, 0, 0), voxel, VOXEL_FACE_LF))
                        mask &= ~VOXEL_FACE_RT;
                    if((mask & VOXEL_FACE_BK) && isOccupied(data, lp + localpos_t(0, 0, 1), voxel, VOXEL_FACE_FT))
                        mask &= ~VOXEL_FACE_BK;
                    if((mask & VOXEL_FACE_FT) && isOccupied(data, lp - localpos_t(0, 0, 1), voxel, VOXEL_FACE_BK))
                        mask &= ~VOXEL_FACE_FT;
                    if((mask & VOXEL_FACE_UP) && isOccupied(data, lp + localpos_t(0, 1, 0), voxel, VOXEL_FACE_DN))
                        mask &= ~VOXEL_FACE_UP;
                    if((mask & VOXEL_FACE_DN) && isOccupied(data, lp - localpos_t(0, 1, 0), voxel, VOXEL_FACE_UP))
                        mask &= ~VOXEL_FACE_DN;
                    pushFace(builder, node, lp, mask, base);
                }
            }
        }
    }
}

// UNDONE: better greedy meshing?
void voxel_mesher::update()
{
    auto group = globals::registry.group<NeedsVoxelMeshComponent>(entt::get<ChunkComponent>);
    for(auto [entity, chunk] : group.each()) {
        globals::registry.remove<VoxelMeshComponent>(entity);

        chunkpos_t cpl = chunk.position;
        voxel_array_t voxels;

        MesherData data;
        data.self_pos = cpl;

        // Current chunk
        std::copy(chunk.data.cbegin(), chunk.data.cend(), voxels.begin());
        data.data[cpl] = voxels;
        data.self_data = data.data.find(cpl); // should always succeed.

        // Neighbours
        data.trySetChunk(cpl + chunkpos_t(1, 0, 0));
        data.trySetChunk(cpl - chunkpos_t(1, 0, 0));
        data.trySetChunk(cpl + chunkpos_t(0, 1, 0));
        data.trySetChunk(cpl - chunkpos_t(0, 1, 0));
        data.trySetChunk(cpl + chunkpos_t(0, 0, 1));
        data.trySetChunk(cpl - chunkpos_t(0, 0, 1));

        ChunkMeshBuilder builder;
        genMesh(builder, data);

        if(!builder.empty()) {
            VoxelMeshComponent &mesh = globals::registry.emplace_or_replace<VoxelMeshComponent>(entity);

            uvre::BufferInfo ibo_info = {};
            ibo_info.type = uvre::BufferType::INDEX_BUFFER;
            ibo_info.size = builder.isize();
            ibo_info.data = builder.idata();

            uvre::BufferInfo vbo_info = {};
            vbo_info.type = uvre::BufferType::VERTEX_BUFFER;
            vbo_info.size = builder.vsize();
            vbo_info.data = builder.vdata();

            mesh.ibo = globals::render_device->createBuffer(ibo_info);
            mesh.vbo = globals::render_device->createBuffer(vbo_info);
            mesh.count = builder.icount();
        }

        // Un-flag the entity
        globals::registry.remove<NeedsVoxelMeshComponent>(entity);
    }
}
