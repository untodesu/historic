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
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/util/mesh_builder.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <thread_pool.hpp>

struct MesherData final {
    using map_type = std::unordered_map<chunkpos_t, ClientChunk>;
    chunkpos_t self_pos;
    map_type::const_iterator self_data;
    map_type data;

    void trySetChunk(const chunkpos_t &cp)
    {
        ClientChunk *chunk = cl_globals::chunks.find(cp);
        if(!chunk)
            return;
        data[cp] = *chunk;
    }
};

using ChunkMeshBuilder = MeshBuilder<uint16_t, PackedVertex>;

static inline void pushQuad(ChunkMeshBuilder *builder, uint16_t &base, const PackedVertex data[4])
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

static inline bool isOccupied(const chunkpos_t &cp, const localpos_t &lp, voxel_t current, const VoxelInfo &current_info, VoxelFace face)
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    if(voxel_t compare = cl_globals::chunks.get(vp)) {
        if(compare != current) {
            if(const VoxelInfo *info = cl_globals::voxels.tryGet(compare))
                return (info->transparency.find(face) == info->transparency.cend());
            return false;
        }

        return (current_info.transparency.find(face) == current_info.transparency.cend());
    }

    return false;
}

static void greedyFace(ChunkMeshBuilder *builder, const chunkpos_t &cp, const VoxelInfo &info, const AtlasNode *node, voxel_t voxel, VoxelFace face, uint16_t &base)
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
                // NOTE: the neighbouring voxel's face
                // is "inverted" (means LF becomes RT etc.)
                mask[maskpos++] = cl_globals::chunks.get(cp, x) == voxel && isOccupied(cp, x, voxel, info, face) && !isOccupied(cp, x + q, voxel, info, back_face);
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

                    PackedVertex verts[4];
                    if(isBackVoxelFace(face)) {
                        verts[0] = PackedVertex(position, texcoords[0], node->index);
                        verts[1] = PackedVertex(position + dv, texcoords[1], node->index);
                        verts[2] = PackedVertex(position + du + dv, texcoords[2], node->index);
                        verts[3] = PackedVertex(position + du, texcoords[3], node->index);
                    }
                    else {
                        verts[0] = PackedVertex(position, texcoords[0], node->index);
                        verts[1] = PackedVertex(position + du, texcoords[1], node->index);
                        verts[2] = PackedVertex(position + du + dv, texcoords[2], node->index);
                        verts[3] = PackedVertex(position + dv, texcoords[3], node->index);
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

// HACK: when shutting down we need
// to cancel all the meshing tasks
static bool cancel_meshing = false;
static thread_pool mesher_pool(16);

static void greedyMesh(ChunkMeshBuilder *builder, const chunkpos_t &cp)
{
    uint16_t base = 0;
    for(VoxelDef::const_iterator it = cl_globals::voxels.cbegin(); it != cl_globals::voxels.cend(); it++) {
        if(it->second.type == VoxelType::SOLID) {
            for(const VoxelFaceInfo &face : it->second.faces) {
                if(const AtlasNode *node = cl_globals::solid_textures.getNode(face.texture)) {
                    greedyFace(builder, cp, it->second, node, it->first, face.face, base);
                    if(cancel_meshing) {
                        builder->clear();
                        return;
                    }
                }
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

void voxel_mesher::update()
{
    // Firstly we go through things that require meshing.
    const auto pending_group = cl_globals::registry.group<NeedsVoxelMeshComponent>(entt::get<chunkpos_t>);
    for(const auto [entity, chunkpos] : pending_group.each()) {
        cl_globals::registry.remove<NeedsVoxelMeshComponent>(entity);
        ThreadedVoxelMesherComponent &mesher = cl_globals::registry.emplace_or_replace<ThreadedVoxelMesherComponent>(entity);
        mesher.builder = new ChunkMeshBuilder();
        mesher.future = mesher_pool.submit(greedyMesh, mesher.builder, chunkpos);
    }

    // Secondly we go through finished tasks
    const auto finished_view = cl_globals::registry.view<ThreadedVoxelMesherComponent>();
    for(const auto [entity, mesher] : finished_view.each()) {
        if(mesher.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(!mesher.builder->empty()) {       
                VoxelMeshComponent *mesh = cl_globals::registry.try_get<VoxelMeshComponent>(entity);
                if(!mesh) {
                    mesh = &cl_globals::registry.emplace<VoxelMeshComponent>(entity);
                    mesh->ibo.create();
                    mesh->vbo.create();
                    mesh->vao.create();
                    mesh->cmd.create();
                    mesh->vao.setIndexBuffer(mesh->ibo);
                    mesh->vao.setVertexBuffer(0, mesh->vbo, sizeof(PackedVertex));
                    mesh->vao.enableAttribute(0, true);
                    mesh->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 2, offsetof(PackedVertex, pack), false);
                    mesh->vao.setAttributeBinding(0, 0);
                }

                mesh->ibo.resize(mesher.builder->isize(), mesher.builder->idata(), GL_STATIC_DRAW);
                mesh->vbo.resize(mesher.builder->vsize(), mesher.builder->vdata(), GL_STATIC_DRAW);
                mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, mesher.builder->icount(), 1, 0, 0, 0);
            }

            delete mesher.builder;
            cl_globals::registry.remove<ThreadedVoxelMesherComponent>(entity);
        }
    }
}
