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

// TODO: move this somewhere else
enum class VoxelFace {
    LEFT,   // 
    RIGHT,  // 
    BACK,   // 
    FRONT,  // 
    TOP,    // 
    BOTTOM  // 
};

using ChunkMeshBuilder = MeshBuilder<uvre::Index32, Vertex>;

static void pushFace(ChunkMeshBuilder &builder, const localpos_t &lp, VoxelFace face, uvre::Index32 &base)
{
    const float3_t lpf = float3_t(lp);

    switch(face) {
        case VoxelFace::LEFT:
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f) });
            break;
        case VoxelFace::RIGHT:
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f) });
            break;
        case VoxelFace::BACK:
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f) });
            break;
        case VoxelFace::FRONT:
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f) });
            break;
        case VoxelFace::TOP:
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 1.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 1.0f, 1.0f) });
            break;
        case VoxelFace::BOTTOM:
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 0.0f) });
            builder.push(Vertex { lpf + float3_t(1.0f, 0.0f, 1.0f) });
            builder.push(Vertex { lpf + float3_t(0.0f, 0.0f, 1.0f) });
            break;
    }

    builder.push(base + 0);
    builder.push(base + 1);
    builder.push(base + 2);
    builder.push(base + 2);
    builder.push(base + 3);
    builder.push(base + 0);
    base += 4;
}

static void genMesh(const ChunkComponent &chunk, voxel_t voxel, ChunkMeshBuilder &builder)
{
    uvre::Index32 base = 0;
    for(voxelidx_t i = 0; i < CHUNK_VOLUME; i++) {
        if(chunk.data[i] == voxel) {
            const localpos_t lp = toLocalPos(i);
            pushFace(builder, lp, VoxelFace::LEFT, base);
            pushFace(builder, lp, VoxelFace::RIGHT, base);
            //pushFace(builder, lp, VoxelFace::BACK, base);
            //pushFace(builder, lp, VoxelFace::FRONT, base);
            //pushFace(builder, lp, VoxelFace::TOP, base);
            //pushFace(builder, lp, VoxelFace::BOTTOM, base);
        }
    }
}

// UNDONE: better greedy meshing?
void voxel_mesher::update()
{
    entt::registry &registry = client_world::registry();
    auto group = registry.group<NeedsVoxelMeshComponent>(entt::get<ChunkComponent>);
    for(auto [entity, chunk] : group.each()) {
        ChunkMeshBuilder builder;
        genMesh(chunk, 0xFF, builder);
        
        // We have the component already.
        if(VoxelMeshComponent *mesh = registry.try_get<VoxelMeshComponent>(entity)) {
            for(VoxelMesh &part : mesh->data) {
                globals::render_device->resizeBuffer(part.ibo, builder.calcIBOSize(), builder.getIndices());
                globals::render_device->resizeBuffer(part.vbo, builder.calcVBOSize(), builder.getVertices());
                part.count = builder.numIndices();
            }

            continue;
        }

        uvre::BufferInfo ibo_info = {};
        ibo_info.type = uvre::BufferType::INDEX_BUFFER;
        ibo_info.size = builder.calcIBOSize();
        ibo_info.data = builder.getIndices();

        uvre::BufferInfo vbo_info = {};
        vbo_info.type = uvre::BufferType::VERTEX_BUFFER;
        vbo_info.size = builder.calcVBOSize();
        vbo_info.data = builder.getVertices();

        VoxelMeshComponent &comp = registry.emplace<VoxelMeshComponent>(entity);
        comp.data.clear();
        VoxelMesh part = {};
        part.ibo = globals::render_device->createBuffer(ibo_info);
        part.vbo = globals::render_device->createBuffer(vbo_info);
        part.texture = res::load<uvre::Texture>("textures/test.jpg", 0xAB);
        part.count = builder.numIndices();
        comp.data.push_back(part);

        // Un-flag the entity
        registry.remove<NeedsVoxelMeshComponent>(entity);
    }
}
