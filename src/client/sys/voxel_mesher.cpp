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

static inline void pushQuad(ChunkMeshBuilder *builder, uint16_t &base, const VoxelVertex data[4])
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

static inline bool isOccupied(const chunkpos_t &cp, const localpos_t &lp, voxel_t current, const VoxelInfo &current_info, voxel_face_t face)
{
    const voxelpos_t vp = toVoxelPos(cp, lp);
    if(const voxel_array_t *chunk = globals::chunks.find(toChunkPos(vp))) {
        const voxel_t compare = chunk->at(toVoxelIdx(toLocalPos(vp)));
        if(compare != current) {
            const VoxelInfo *info = globals::voxels.tryGet(compare);
            return info ? !(info->transparency & face) : false;
        }

        return !(current_info.transparency & face);
    }

    return false;
}

static inline const int faceNormalInv(voxel_face_t face)
{
    if(face & VOXEL_FACE_LF)
        return -1;
    if(face & VOXEL_FACE_FT)
        return -1;
    if(face & VOXEL_FACE_DN)
        return -1;
    if(face & VOXEL_FACE_RT)
        return 1;
    if(face & VOXEL_FACE_BK)
        return 1;
    if(face & VOXEL_FACE_UP)
        return 1;
    return 0;
}

static void greedyFace(ChunkMeshBuilder *builder, const chunkpos_t &cp, const VoxelInfo &info, const AtlasNode *node, voxel_t voxel, voxel_face_t face, uint16_t &base)
{
    const voxel_face_t back_face = backVoxelFace(face);

    std::array<bool, CHUNK_AREA> mask;

    int16_t d = 0;
    if(face & (VOXEL_FACE_LF | VOXEL_FACE_RT))
        d = 0; // x
    else if(face & (VOXEL_FACE_UP | VOXEL_FACE_DN))
        d = 1; // y
    else if(face & (VOXEL_FACE_FT | VOXEL_FACE_BK))
        d = 2; // z
    else
        return;
    const int16_t u = (d + 1) % 3;
    const int16_t v = (d + 2) % 3;
    localpos_t x = localpos_t(0, 0, 0);
    localpos_t q = localpos_t(0, 0, 0);
    q[d] = faceNormalInv(face);

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
                mask[maskpos++] = isOccupied(cp, x, voxel, info, face) && !isOccupied(cp, x + q, voxel, info, back_face);
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
                    for(qw = 1; mask[maskpos + qw] && (i + qw) < CHUNK_SIZE_I16; qw++);

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

                    // HACK
                    const bool is_back_face = face & (VOXEL_FACE_LF | VOXEL_FACE_FT | VOXEL_FACE_DN);

                    float2_t uv = float2_t(qw, qh) * node->max_uv;
                    float3_t pos = float3_t(x.x, x.y, x.z);
                    float3_t du = FLOAT3_ZERO;
                    float3_t dv = FLOAT3_ZERO;
                    du[u] = static_cast<float>(qw);
                    dv[v] = static_cast<float>(qh);

                    // HACK
                    if(is_back_face)
                        pos[d] += q[d];

                    float2_t uvs[4];
                    if(face & VOXEL_FACE_LF) {
                        uvs[0] = float2_t(0.0f, 0.0f);
                        uvs[1] = float2_t(uv.y, 0.0f);
                        uvs[2] = float2_t(uv.y, uv.x);
                        uvs[3] = float2_t(0.0f, uv.x);
                    }
                    else if(face & VOXEL_FACE_RT) {
                        uvs[0] = float2_t(uv.y, 0.0f);
                        uvs[1] = float2_t(uv.y, uv.x);
                        uvs[2] = float2_t(0.0f, uv.x);
                        uvs[3] = float2_t(0.0f, 0.0f);
                    }
                    else if(face & VOXEL_FACE_FT) {
                        uvs[0] = float2_t(uv.x, 0.0f);
                        uvs[1] = float2_t(uv.x, uv.y);
                        uvs[2] = float2_t(0.0f, uv.y);
                        uvs[3] = float2_t(0.0f, 0.0f);
                    }
                    else if(face & VOXEL_FACE_BK) {
                        uvs[0] = float2_t(0.0f, 0.0f);
                        uvs[1] = float2_t(uv.x, 0.0f);
                        uvs[2] = float2_t(uv.x, uv.y);
                        uvs[3] = float2_t(0.0f, uv.y);
                    }
                    else if(face & VOXEL_FACE_UP) {
                        uvs[0] = float2_t(0.0f, uv.x);
                        uvs[1] = float2_t(0.0f, 0.0f);
                        uvs[2] = float2_t(uv.y, 0.0f);
                        uvs[3] = float2_t(uv.y, uv.x);
                    }
                    else if(face & VOXEL_FACE_DN) {
                        uvs[0] = float2_t(uv.y, uv.x);
                        uvs[1] = float2_t(0.0f, uv.x);
                        uvs[2] = float2_t(0.0f, 0.0f);
                        uvs[3] = float2_t(uv.y, 0.0f);
                    }

                    VoxelVertex verts[4];
                    if(is_back_face) {
                        verts[0] = VoxelVertex(pos, uvs[0], node->index);
                        verts[1] = VoxelVertex(pos + dv, uvs[1], node->index);
                        verts[2] = VoxelVertex(pos + du + dv, uvs[2], node->index);
                        verts[3] = VoxelVertex(pos + du, uvs[3], node->index);
                    }
                    else {
                        verts[0] = VoxelVertex(pos, uvs[0], node->index);
                        verts[1] = VoxelVertex(pos + du, uvs[1], node->index);
                        verts[2] = VoxelVertex(pos + du + dv, uvs[2], node->index);
                        verts[3] = VoxelVertex(pos + dv, uvs[3], node->index);
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
                    continue;
                }

                i++;
                maskpos++;
            }
        }
    }
}

// HACK: when shutting down we need
// to cancel all the meshing tasks
static bool cancel_meshing = false;
static thread_pool mesher_pool(8);

static void greedyMesh(ChunkMeshBuilder *builder, const chunkpos_t &cp)
{
    uint16_t base = 0;
    std::vector<voxel_face_t> unwrap;
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        if(it->second.type == VoxelType::SOLID) {
            for(const VoxelFaceInfo &face : it->second.faces) {
                unwrapVoxelFaces(face.mask, unwrap);
                if(const AtlasNode *node = globals::solid_textures.getNode(face.texture)) {
                    for(const voxel_face_t &uface : unwrap) {
                        greedyFace(builder, cp, it->second, node, it->first, uface, base);
                        if(cancel_meshing) {
                            builder->clear();
                            return;
                        }
                    }
                }
            }
        }
    }
}

// SAVED: thais code will be used for
// SOLID_FLORA voxels when I will implement 'em.
#if 0
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
                        if((mask & VOXEL_FACE_LF) && isOccupied(cp, lp - localpos_t(1, 0, 0), voxel, *info, VOXEL_FACE_RT))
                            mask &= ~VOXEL_FACE_LF;
                        if((mask & VOXEL_FACE_RT) && isOccupied(cp, lp + localpos_t(1, 0, 0), voxel, *info, VOXEL_FACE_LF))
                            mask &= ~VOXEL_FACE_RT;
                        if((mask & VOXEL_FACE_BK) && isOccupied(cp, lp + localpos_t(0, 0, 1), voxel, *info, VOXEL_FACE_FT))
                            mask &= ~VOXEL_FACE_BK;
                        if((mask & VOXEL_FACE_FT) && isOccupied(cp, lp - localpos_t(0, 0, 1), voxel, *info, VOXEL_FACE_BK))
                            mask &= ~VOXEL_FACE_FT;
                        if((mask & VOXEL_FACE_UP) && isOccupied(cp, lp + localpos_t(0, 1, 0), voxel, *info, VOXEL_FACE_DN))
                            mask &= ~VOXEL_FACE_UP;
                        if((mask & VOXEL_FACE_DN) && isOccupied(cp, lp - localpos_t(0, 1, 0), voxel, *info, VOXEL_FACE_UP))
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
#endif

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
    const auto pending_group = globals::registry.group<NeedsVoxelMeshComponent>(entt::get<chunkpos_t>);
    for(const auto [entity, chunkpos] : pending_group.each()) {
        globals::registry.remove<NeedsVoxelMeshComponent>(entity);
        ThreadedVoxelMesherComponent &mesher = globals::registry.emplace_or_replace<ThreadedVoxelMesherComponent>(entity);
        mesher.builder = new ChunkMeshBuilder();
        mesher.future = mesher_pool.submit(greedyMesh, mesher.builder, chunkpos);
    }

    // Secondly we go through finished tasks
    const auto finished_view = globals::registry.view<ThreadedVoxelMesherComponent>();
    for(const auto [entity, mesher] : finished_view.each()) {
        if(mesher.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(!mesher.builder->empty()) {       
                VoxelMeshComponent *mesh = globals::registry.try_get<VoxelMeshComponent>(entity);
                if(!mesh) {
                    mesh = &globals::registry.emplace<VoxelMeshComponent>(entity);
                    mesh->ibo.create();
                    mesh->vbo.create();
                    mesh->vao.create();
                    mesh->cmd.create();
                    mesh->vao.setIndexBuffer(mesh->ibo);
                    mesh->vao.setVertexBuffer(0, mesh->vbo, sizeof(VoxelVertex));
                    mesh->vao.enableAttribute(0, true);
                    mesh->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 2, offsetof(VoxelVertex, pack), false);
                    mesh->vao.setAttributeBinding(0, 0);
                }

                mesh->ibo.resize(mesher.builder->isize(), mesher.builder->idata(), GL_STATIC_DRAW);
                mesh->vbo.resize(mesher.builder->vsize(), mesher.builder->vdata(), GL_STATIC_DRAW);
                mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, mesher.builder->icount(), 1, 0, 0, 0);
            }

            delete mesher.builder;
            globals::registry.remove<ThreadedVoxelMesherComponent>(entity);
        }
    }
}
