/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/client_globals.hpp>
#include <client/packed_vertex.hpp>
#include <client/client_world.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/res.hpp>
#include <uvre/uvre.hpp>

class MeshBuilder final {
public:
    void push(const Vertex &vtx);
    uvre::Index32 base = 0;
    std::vector<uvre::Index32> indices;
    std::vector<Vertex> vertices;
};

void MeshBuilder::push(const Vertex &vtx)
{
    indices.push_back(base++);
    vertices.push_back(vtx);
}

// UNDONE: better greedy meshing?
void voxel_mesher::update()
{
    entt::registry &registry = client_world::registry();
    auto group = registry.group<NeedsVoxelMeshComponent>(entt::get<ChunkComponent>);
    for(auto [entity, chunk] : group.each()) {
        MeshBuilder builder;

        for(voxelidx_t i = 0; i < CHUNK_VOLUME; i++) {
            if(chunk.data[i] == 0xFF) {
                float3_t lp = float3_t(toLocalPos(i));
                builder.push(Vertex { lp + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) });
                builder.push(Vertex { lp + float3_t(1.0f, 0.0f, 0.0f), float2_t(1.0f, 0.0f) });
                builder.push(Vertex { lp + float3_t(1.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) });
                builder.push(Vertex { lp + float3_t(1.0f, 1.0f, 0.0f), float2_t(1.0f, 1.0f) });
                builder.push(Vertex { lp + float3_t(0.0f, 1.0f, 0.0f), float2_t(0.0f, 1.0f) });
                builder.push(Vertex { lp + float3_t(0.0f, 0.0f, 0.0f), float2_t(0.0f, 0.0f) });
            }
        }

        // We have the component already.
        if(VoxelMeshComponent *mesh = registry.try_get<VoxelMeshComponent>(entity)) {
            for(VoxelMesh &part : mesh->data) {
                globals::render_device->resizeBuffer(part.ibo, sizeof(uvre::Index16) * builder.indices.size(), builder.indices.data());
                globals::render_device->resizeBuffer(part.vbo, sizeof(Vertex) * builder.vertices.size(), builder.vertices.data());
                part.count = builder.indices.size();
            }

            continue;
        }

        uvre::BufferInfo ibo_info = {};
        ibo_info.type = uvre::BufferType::INDEX_BUFFER;
        ibo_info.size = sizeof(uvre::Index32) * builder.indices.size();
        ibo_info.data = builder.indices.data();

        uvre::BufferInfo vbo_info = {};
        vbo_info.type = uvre::BufferType::VERTEX_BUFFER;
        vbo_info.size = sizeof(Vertex) * builder.vertices.size();
        vbo_info.data = builder.vertices.data();

        VoxelMeshComponent &comp = registry.emplace<VoxelMeshComponent>(entity);
        comp.data.clear();
        VoxelMesh part = {};
        part.ibo = globals::render_device->createBuffer(ibo_info);
        part.vbo = globals::render_device->createBuffer(vbo_info);
        part.texture = res::load<uvre::Texture>("textures/test.jpg", 0xAB);
        part.count = builder.indices.size();
        comp.data.push_back(part);

        registry.remove<NeedsVoxelMeshComponent>(entity);
    }
}
