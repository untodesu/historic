/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
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

struct ChunkMesherData final {
    std::unordered_map<chunkpos_t, ClientChunk> data;
    const size_t trySetChunk(const chunkpos_t &cp);
    const bool hasVoxel(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel);
    const bool hasFace(const chunkpos_t &cp, const localpos_t &lp, VoxelFace face);
};

inline const size_t ChunkMesherData::trySetChunk(const chunkpos_t &cp)
{
    if(ClientChunk *chunk = globals::chunks.find(cp)) {
        data[cp] = *chunk;
        return CHUNK_VOLUME;
    }

    return 0;
}

inline const bool ChunkMesherData::hasVoxel(const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel)
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    const auto it = data.find(toChunkPos(vp));
    if(it != data.cend())
        return it->second.data.at(toVoxelIdx(toLocalPos(vp))) == voxel;
    return false;
}

inline const bool ChunkMesherData::hasFace(const chunkpos_t &cp, const localpos_t &lp, VoxelFace face)
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    const auto it = data.find(toChunkPos(vp));
    if(it != data.cend()) {
        if(voxel_t voxel = it->second.data.at(toVoxelIdx(toLocalPos(vp)))) {
            if(const VoxelInfo *info = globals::voxels.tryGet(voxel))
                return info->transparency.find(face) == info->transparency.cend();
            return false;
        }
    }

    return false;
}

using ChunkMeshBuilder = MeshBuilder<uint16_t, Vertex>;
static inline void pushQuad(ChunkMeshBuilder *builder, uint16_t &base, const Vertex data[4])
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

static void greedyFace(ChunkMeshBuilder *builder, ChunkMesherData *data, const chunkpos_t &cp, const VoxelInfo &info, const AtlasNode *node, voxel_t voxel, VoxelFace face, uint16_t &base)
{
    const VoxelFace back_face = backVoxelFace(face);

    std::array<bool, CHUNK_AREA> mask;

    int16_t d = 0;
    switch(face) {
        case VoxelFace::LF:
        case VoxelFace::RT:
            d = 0;
            break;
        case VoxelFace::UP:
        case VoxelFace::DN:
            d = 1;
            break;
        case VoxelFace::FT:
        case VoxelFace::BK:
            d = 2;
            break;
        default:
            return;
    }

    const int16_t u = (d + 1) % 3;
    const int16_t v = (d + 2) % 3;
    localpos_t x = localpos_t(0, 0, 0);
    localpos_t q = localpos_t(0, 0, 0);
    q[d] = -voxelFaceNormal(face);

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
                mask[maskpos++] = data->hasVoxel(cp, x, voxel) && !data->hasFace(cp, x + q, back_face);
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
                    if(isBackVoxelFace(face))
                        position[d] += q[d];

                    float2 texcoords[4];
                    float2 tc = float2(qw, qh) * node->max_uv;
                    switch(face) {
                        case VoxelFace::LF:
                            texcoords[0] = float2(0.0f, 0.0f);
                            texcoords[1] = float2(tc.y, 0.0f);
                            texcoords[2] = float2(tc.y, tc.x);
                            texcoords[3] = float2(0.0f, tc.x);
                            break;
                        case VoxelFace::RT:
                            texcoords[0] = float2(tc.y, 0.0f);
                            texcoords[1] = float2(tc.y, tc.x);
                            texcoords[2] = float2(0.0f, tc.x);
                            texcoords[3] = float2(0.0f, 0.0f);
                            break;
                        case VoxelFace::FT:
                            texcoords[0] = float2(tc.x, 0.0f);
                            texcoords[1] = float2(tc.x, tc.y);
                            texcoords[2] = float2(0.0f, tc.y);
                            texcoords[3] = float2(0.0f, 0.0f);
                            break;
                        case VoxelFace::BK:
                            texcoords[0] = float2(0.0f, 0.0f);
                            texcoords[1] = float2(tc.x, 0.0f);
                            texcoords[2] = float2(tc.x, tc.y);
                            texcoords[3] = float2(0.0f, tc.y);
                            break;
                        case VoxelFace::UP:
                            texcoords[0] = float2(0.0f, tc.x);
                            texcoords[1] = float2(0.0f, 0.0f);
                            texcoords[2] = float2(tc.y, 0.0f);
                            texcoords[3] = float2(tc.y, tc.x);
                            break;
                        case VoxelFace::DN:
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
                    if(isBackVoxelFace(face)) {
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

static bool cancel_meshing = false;
static thread_pool mesher_pool(9);
static size_t meshing_memory = 0;

// UNDONE 0: translucency (second GBuffer for it?).
// UNDONE 1: naive meshing for SOLID_FLORA voxels.
// UNDONE 2: marching cubes (I guess) for LIQUID voxels would look awesome.
static void genMesh(ChunkMeshBuilder *builder, ChunkMesherData *data, const chunkpos_t &cp)
{
    uint16_t base = 0;
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        if(it->second.type == VoxelType::SOLID) {
            for(const VoxelFaceInfo &face : it->second.faces) {
                if(const AtlasNode *node = globals::solid_textures.getNode(face.texture)) {
                    greedyFace(builder, data, cp, it->second, node, it->first, face.face, base);
                    if(cancel_meshing) {
                        builder->clear();
                        return;
                    }
                }
            }
        }
    }
}

struct ThreadedChunkMesherComponent final {
    ChunkMeshBuilder *builder { nullptr };
    ChunkMesherData *data { nullptr };
    std::shared_future<bool> future;
};

void chunk_mesher::shutdown()
{
    cancel_meshing = true;
    mesher_pool.wait_for_tasks();
}

void chunk_mesher::update()
{
    // Firstly we go through things that require meshing.
    const auto pending_group = globals::registry.group<ChunkFlaggedForMeshingComponent>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : pending_group.each()) {
        globals::registry.remove<ChunkFlaggedForMeshingComponent>(entity);
        if(ClientChunk *client_chunk = globals::chunks.find(chunk.position)) {
            ThreadedChunkMesherComponent &mesher = globals::registry.emplace_or_replace<ThreadedChunkMesherComponent>(entity);
            mesher.builder = new ChunkMeshBuilder();
            mesher.data = new ChunkMesherData();

            meshing_memory += sizeof(ChunkMeshBuilder);
            meshing_memory += sizeof(ChunkMesherData);

            meshing_memory += mesher.data->trySetChunk(chunk.position);
            meshing_memory += mesher.data->trySetChunk(chunk.position + chunkpos_t(0, 0, 1));
            meshing_memory += mesher.data->trySetChunk(chunk.position - chunkpos_t(0, 0, 1));
            meshing_memory += mesher.data->trySetChunk(chunk.position + chunkpos_t(0, 1, 0));
            meshing_memory += mesher.data->trySetChunk(chunk.position - chunkpos_t(0, 1, 0));
            meshing_memory += mesher.data->trySetChunk(chunk.position + chunkpos_t(1, 0, 0));
            meshing_memory += mesher.data->trySetChunk(chunk.position - chunkpos_t(1, 0, 0));

            mesher.future = mesher_pool.submit(genMesh, mesher.builder, mesher.data, chunk.position);
        }
    }

    // Secondly we go through finished tasks
    const auto finished_view = globals::registry.view<ThreadedChunkMesherComponent>();
    for(const auto [entity, mesher] : finished_view.each()) {
        if(mesher.future.valid() && mesher.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(!mesher.builder->empty()) {       
                ChunkMeshComponent *mesh = globals::registry.try_get<ChunkMeshComponent>(entity);
                if(!mesh) {
                    mesh = &globals::registry.emplace<ChunkMeshComponent>(entity);
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

                mesh->ibo.resize(mesher.builder->isize(), mesher.builder->idata(), GL_STATIC_DRAW);
                mesh->vbo.resize(mesher.builder->vsize(), mesher.builder->vdata(), GL_STATIC_DRAW);
                mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, mesher.builder->icount(), 1, 0, 0, 0);
            }

            meshing_memory -= mesher.data->data.size() * CHUNK_VOLUME;
            meshing_memory -= sizeof(ChunkMesherData);
            meshing_memory -= sizeof(ChunkMeshBuilder);

            delete mesher.data;
            delete mesher.builder;

            globals::registry.remove<ThreadedChunkMesherComponent>(entity);
        }
    }
}

size_t chunk_mesher::memory()
{
    return meshing_memory;
}
