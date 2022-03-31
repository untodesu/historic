/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/comp/chunk_mesh_component.hpp>
#include <client/globals.hpp>
#include <client/mesh_builder.hpp>
#include <client/terrain_mesher.hpp>
#include <common/comp/chunk_component.hpp>
#include <common/math/constexpr.hpp>
#include <common/math/crc64.hpp>
#include <common/voxels.hpp>
#include <random>

struct TerrainVertex final {
    // ID: 0.
    // Type: 32-bit unsigned integer (5_5_5_1_8_8).
    // Data: position and texture coordinates.
    GLuint pack_0;

    // ID: 1.
    // Type: 32-bit integer (2_10_10_10).
    // Data: normal vector and shade.
    GLuint pack_1;

    // ID: 2.
    // Type: 32-bit integer.
    // Data: atlas entry index.
    GLuint pack_2;

    TerrainVertex(const local_pos_t &position, const vector2f_t &texcoord, int shade, const vector3f_t &normal, GLuint atlas_entry_index)
        : pack_0(0), pack_1(0), pack_2(atlas_entry_index)
    {
        pack_0 |= (math::clamp<int16_t>(position.x, 0, 16) & 0x1F) << 27;
        pack_0 |= (math::clamp<int16_t>(position.y, 0, 16) & 0x1F) << 22;
        pack_0 |= (math::clamp<int16_t>(position.z, 0, 16) & 0x1F) << 17;
        pack_0 |= (glm::packUnorm4x8(vector4f_t(texcoord.x, texcoord.y, 0.0f, 0.0f)) & 0xFFFF);
        pack_1 |= (static_cast<GLuint>(shade) & 0x03) << 30;
        pack_1 |= (static_cast<GLuint>((math::clamp(normal.z, -1.0f, 1.0f) + 1.0f) * 0.5f * 1023.0f) & 0x3FF) << 20;
        pack_1 |= (static_cast<GLuint>((math::clamp(normal.y, -1.0f, 1.0f) + 1.0f) * 0.5f * 1023.0f) & 0x3FF) << 10;
        pack_1 |= (static_cast<GLuint>((math::clamp(normal.x, -1.0f, 1.0f) + 1.0f) * 0.5f * 1023.0f) & 0x3FF);
    }
};

using TerrainMeshBuilder = MeshBuilder<GLushort, TerrainVertex>;

static void pushFace(TerrainMeshBuilder &builder, const AtlasEntry *entry, const local_pos_t &lpos, VoxelFace face, GLushort &base)
{
    if(face == VoxelFace::LF) {
        const vector3f_t face_normal = vector3f_t(-1.0f, 0.0f, 0.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 0), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 1), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 1), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 0), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }

    if(face == VoxelFace::RT) {
        const vector3f_t face_normal = vector3f_t(1.0f, 0.0f, 0.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 0), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 0), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 1), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 1), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 1, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }

    if(face == VoxelFace::FT) {
        const vector3f_t face_normal = vector3f_t(0.0f, 0.0f, 1.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 0), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 0), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 0), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 0), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }

    if(face == VoxelFace::BK) {
        const vector3f_t face_normal = vector3f_t(0.0f, 0.0f, -1.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 1), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 1), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 1), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 1), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 2, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }

    if(face == VoxelFace::UP) {
        const vector3f_t face_normal = vector3f_t(0.0f, 1.0f, 0.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 0), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 3, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 1, 1), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 3, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 1), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 3, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 1, 0), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 3, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }

    if(face == VoxelFace::DN) {
        const vector3f_t face_normal = vector3f_t(0.0f, -1.0f, 0.0f);
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 0), vector2f_t(0.0f, 0.0f) * entry->max_texcoord, 0, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 0), vector2f_t(1.0f, 0.0f) * entry->max_texcoord, 0, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(1, 0, 1), vector2f_t(1.0f, 1.0f) * entry->max_texcoord, 0, face_normal, entry->entry_index));
        builder.vertex(TerrainVertex(lpos + local_pos_t(0, 0, 1), vector2f_t(0.0f, 1.0f) * entry->max_texcoord, 0, face_normal, entry->entry_index));
        builder.index(base + 0);
        builder.index(base + 1);
        builder.index(base + 2);
        builder.index(base + 2);
        builder.index(base + 3);
        builder.index(base + 0);
        base += 4;
        return;
    }
}

static inline bool isOccupied(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t compare, VoxelFace face)
{
    const voxel_pos_t &vpos = world::getVoxelPosition(cpos, lpos);
    if(const ClientChunk *chunk = globals::chunks.find(world::getChunkPosition(vpos))) {
        const voxel_t voxel = chunk->data.at(world::getVoxelIndex(world::getLocalPosition(vpos)));
        if(compare != voxel) {
            if(const VoxelDefEntry *def = globals::voxels.find(voxel)) {
                const auto def_face = def->faces.find(face);
                if(def_face != def->faces.cend())
                    return !def_face->second.transparent;
                return false;
            }

            return false;
        }

        return true;
    }

    return false;
}

static void genCubeMesh(TerrainMeshBuilder &builder, const chunk_pos_t &cpos)
{
    GLushort base = 0;
    if(const ClientChunk *chunk = globals::chunks.find(cpos)) {
        for(voxel_idx_t i = 0; i < CHUNK_VOLUME; i++) {
            const voxel_t voxel = chunk->data.at(i);
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

                        if(isOccupied(cpos, neighbour_lpos, voxel, neighbour_face))
                            continue;
                        pushFace(builder, entry, lpos, def_face.first, base);
                    }
                }
            }
        }
    }
}

void terrain_mesher::update()
{
    // FIXME: we should really do this in a separate thread.
    const auto group = globals::registry.group<ChunkQueuedMeshingComponent>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : group.each()) {
        ChunkMeshComponent &meshes = globals::registry.get_or_emplace<ChunkMeshComponent>(entity);

        TerrainMeshBuilder builder;
        genCubeMesh(builder, chunk.cpos);

        ChunkMesh *cube_mesh = meshes.find(VoxelType::STATIC_CUBE);
        if(!cube_mesh) {
            cube_mesh = &(meshes.meshes[VoxelType::STATIC_CUBE] = ChunkMesh());
            cube_mesh->cmd.create();
            cube_mesh->ibo.create();
            cube_mesh->vao.create();
            cube_mesh->vbo.create();

            cube_mesh->vao.setIndexBuffer(cube_mesh->ibo);
            cube_mesh->vao.setVertexBuffer(0, cube_mesh->vbo, sizeof(TerrainVertex));

            cube_mesh->vao.enableAttribute(0, true);
            cube_mesh->vao.enableAttribute(1, true);
            cube_mesh->vao.enableAttribute(2, true);

            cube_mesh->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 1, offsetof(TerrainVertex, pack_0), false);
            cube_mesh->vao.setAttributeFormat(1, GL_UNSIGNED_INT, 1, offsetof(TerrainVertex, pack_1), false);
            cube_mesh->vao.setAttributeFormat(2, GL_UNSIGNED_INT, 1, offsetof(TerrainVertex, pack_2), false);

            cube_mesh->vao.setAttributeBinding(0, 0);
            cube_mesh->vao.setAttributeBinding(1, 0);
            cube_mesh->vao.setAttributeBinding(2, 0);
        }

        cube_mesh->ibo.resize(builder.isize(), builder.idata(), GL_STATIC_DRAW);
        cube_mesh->vbo.resize(builder.vsize(), builder.vdata(), GL_STATIC_DRAW);
        cube_mesh->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, builder.icount(), 1, 0, 0, 0);

        globals::registry.remove<ChunkQueuedMeshingComponent>(entity);
        break;
    }
}
