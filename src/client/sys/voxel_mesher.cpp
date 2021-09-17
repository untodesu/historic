/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/globals.hpp>
#include <client/world.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/res.hpp>
#include <uvre/uvre.hpp>
#include <math/vertex.hpp>
#include <client/mesh_builder.hpp>
#include <shared/voxel_def.hpp>
#include <shared/res.hpp>
#include <client/util.hpp>
#include <spdlog/spdlog.h>

using ChunkMeshBuilder = MeshBuilder<uvre::Index32, Vertex>;

// A lot of copypasta. Too bad!
static void pushFace(ChunkMeshBuilder &builder, const localpos_t &lp, voxel_face_t face, uvre::Index32 &base)
{
    const float3_t lpf = float3_t(lp);
    uvre::Index32 num_faces = 0;

    if(face & VOXEL_FACE_LF) {
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_RT) {
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_BK) {
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_FT) {
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_UP) {
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f), float2_t(0.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f), float2_t(0.0f, 0.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }

    if(face & VOXEL_FACE_DN) {
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) });
        builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f), float2_t(1.0f, 1.0f) });
        builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f), float2_t(0.0f, 1.0f) });
        builder.push(base + 0);
        builder.push(base + 1);
        builder.push(base + 2);
        builder.push(base + 2);
        builder.push(base + 3);
        builder.push(base + 0);
        base += 4;
    }
}

static bool isOccupied(const ChunkComponent &chunk, const localpos_t &lp, voxel_face_t face)
{
    const voxel_t compare = isLocalPosInRange(lp) ? chunk.data[toVoxelIdx(lp)] : client_util::getVoxel(chunk.position, lp);
    const VoxelInfo *info = voxel_def::get(compare);
    return info ? !(info->transparency & face) : false;
}

static void genMesh(VoxelMeshComponent &mesh, const ChunkComponent &chunk, voxel_t voxel, const VoxelInfo &info)
{
    for(const VoxelFaceInfo &face_info : info.faces) {
        uvre::Index32 base = 0;
        ChunkMeshBuilder builder;
        
        for(voxelidx_t i = 0; i < CHUNK_VOLUME; i++) {
            if(chunk.data[i] == voxel) {
                const localpos_t lp = toLocalPos(i);
                voxel_face_t mask = face_info.mask;
                if((mask & VOXEL_FACE_LF) && isOccupied(chunk, lp - localpos_t(1, 0, 0), VOXEL_FACE_RT))
                    mask &= ~VOXEL_FACE_LF;
                if((mask & VOXEL_FACE_RT) && isOccupied(chunk, lp + localpos_t(1, 0, 0), VOXEL_FACE_LF))
                    mask &= ~VOXEL_FACE_RT;
                if((mask & VOXEL_FACE_BK) && isOccupied(chunk, lp + localpos_t(0, 0, 1), VOXEL_FACE_FT))
                    mask &= ~VOXEL_FACE_BK;
                if((mask & VOXEL_FACE_FT) && isOccupied(chunk, lp - localpos_t(0, 0, 1), VOXEL_FACE_BK))
                    mask &= ~VOXEL_FACE_FT;
                if((mask & VOXEL_FACE_UP) && isOccupied(chunk, lp + localpos_t(0, 1, 0), VOXEL_FACE_DN))
                    mask &= ~VOXEL_FACE_UP;
                if((mask & VOXEL_FACE_DN) && isOccupied(chunk, lp - localpos_t(0, 1, 0), VOXEL_FACE_UP))
                    mask &= ~VOXEL_FACE_DN;
                pushFace(builder, lp, mask, base);
            }
        }

        if(!builder.empty()) {
            if(face_info.mask == 48) {
                const size_t nv = builder.numVertices();
                const Vertex *vtxs = builder.getVertices();
                for(size_t x = 0; x < nv; x++)
                    spdlog::debug("{} {} {}", vtxs[x].position.x, vtxs[x].position.y, vtxs[x].position.z);
            }

            uvre::BufferInfo ibo_info = {};
            ibo_info.type = uvre::BufferType::INDEX_BUFFER;
            ibo_info.size = builder.calcIBOSize();
            ibo_info.data = builder.getIndices();

            uvre::BufferInfo vbo_info = {};
            vbo_info.type = uvre::BufferType::VERTEX_BUFFER;
            vbo_info.size = builder.calcVBOSize();
            vbo_info.data = builder.getVertices();

            VoxelMesh submesh = {};
            submesh.ibo = globals::render_device->createBuffer(ibo_info);
            submesh.vbo = globals::render_device->createBuffer(vbo_info);
            submesh.texture = res::load<uvre::Texture>(face_info.texture, res::PRECACHE);
            submesh.count = builder.numIndices();

            mesh.data.push_back(std::move(submesh));

            base = 0;
            builder.clear();
        }
    }
}

// UNDONE: better greedy meshing?
void voxel_mesher::update()
{
    entt::registry &registry = client_world::registry();
    auto group = registry.group<NeedsVoxelMeshComponent>(entt::get<ChunkComponent>);
    for(auto [entity, chunk] : group.each()) {
        VoxelMeshComponent &mesh = registry.emplace_or_replace<VoxelMeshComponent>(entity);
        mesh.data.clear();

        const size_t list_size = voxel_def::listSize();
        const voxel_t *list = voxel_def::list();
        for(size_t i = 0; i < list_size; i++) {
            const VoxelInfo *info = voxel_def::get(list[i]);
            if(!info)
                continue;
            genMesh(mesh, chunk, list[i], *info);
        }

        // Un-flag the entity
        registry.remove<NeedsVoxelMeshComponent>(entity);
    }
}
