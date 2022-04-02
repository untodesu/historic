/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/comp/static_chunk_mesh_component.hpp>
#include <client/comp/static_chunk_mesh_needs_update_component.hpp>
#include <client/globals.hpp>
#include <client/mesh_builder.hpp>
#include <client/terrain_mesher.hpp>
#include <common/comp/chunk_component.hpp>
#include <common/math/constexpr.hpp>
#include <common/math/crc64.hpp>
#include <common/voxels.hpp>

// WHY GREEDY MESHING IS GONE FOR GOOD:
//  First. It doesn't allow us to use "random"
// textures for the same voxel - I've already
// modified the original algorithm to support
// face-based comparison which made it even
// slower than before (if you were running a
// Win32 Debug build you should've felt that),
// adding random textures would be suicidal in
// terms of overall mesher performance.
//  Second. Voxel orientations - in future I want
// to have some voxels (in terms of gameplay those
// should be called blocks) to have different orientation
// than others, that's gonna be based on some kind of
// a value in some kind of an NBT-based taglist.
//  Third. Support for custom voxel models. Minecraft
// does that - you can define your own 3D-ish model
// for the bloxel and the mesher will cull the sides
// when a bloxel's (block/voxel) side is connected
// to an opaque bloxel nearby - I want to have that
// applied to STATIC_CUBE and STATIC_FLORA voxels. Also
// the support for custom models will reduce the amounts
// of pain I will have to go through to add slabs and stairs.

struct PackedVertex final {
    // [0]: 3x10-bit position vector and 1x2-bit shade.
    // [1]: 2x5-bit texture coordinates and 3x7-bit normal.
    // [2]: 1x32-bit texture atlas entry index.
    GLuint pack_0 {0};
    GLuint pack_1 {0};
    GLuint pack_2 {0};

    inline PackedVertex(const vector3f_t &position, const vector2f_t &texcoord, unsigned int shade, const vector3f_t &normal, GLuint atlas_entry_index)
        : pack_0(0), pack_1(0), pack_2(atlas_entry_index)
    {
        pack_0 |= (static_cast<GLuint>(position.x / 16.0f * 512.0f) & 0x3FF) << 22;
        pack_0 |= (static_cast<GLuint>(position.y / 16.0f * 512.0f) & 0x3FF) << 12;
        pack_0 |= (static_cast<GLuint>(position.z / 16.0f * 512.0f) & 0x3FF) << 2;
        pack_0 |= (static_cast<GLuint>(shade) & 0x03);
        pack_1 |= (static_cast<GLuint>(math::max(texcoord.x, 0.0f) * 16.0f) & 0x1F) << 27;
        pack_1 |= (static_cast<GLuint>(math::max(texcoord.y, 0.0f) * 16.0f) & 0x1F) << 22;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.x, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F) << 14;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.y, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F) << 7;
        pack_1 |= (static_cast<GLuint>(math::clamp(normal.z, -1.0f, 1.0f) + 1.0f * 0.5f * 127.0f) & 0x7F);
    }
};

using TerrainMeshBuilder = MeshBuilder<GLushort, PackedVertex>;

// NOTE: this method of meshing will be gone
// as soon as I implement a model format in which
// each face would have the ability to be hidden
// when a specific side of the bloxel is obstructed
// by an another non-transparent bloxel.
static void pushCubeFace(TerrainMeshBuilder &builder, const AtlasEntry *atlas_entry, const local_pos_t &lpos, VoxelFace face, GLushort &base)
{
    vector3f_t face_normal;
    local_pos_t offsets[4];
    vector2f_t texcoords[4];
    unsigned int shade;
    switch(face) {
        case VoxelFace::LF:
            face_normal = vector3f_t(-1.0f, 0.0f, 0.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(0, 0, 1);
            offsets[2] = local_pos_t(0, 1, 1);
            offsets[3] = local_pos_t(0, 1, 0);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            shade = 1;
            break;
        case VoxelFace::RT:
            face_normal = vector3f_t(1.0f, 0.0f, 0.0f);
            offsets[0] = local_pos_t(1, 0, 0);
            offsets[1] = local_pos_t(1, 1, 0);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(1, 0, 1);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            shade = 1;
            break;
        case VoxelFace::FT:
            face_normal = vector3f_t(0.0f, 0.0f, 1.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(0, 1, 0);
            offsets[2] = local_pos_t(1, 1, 0);
            offsets[3] = local_pos_t(1, 0, 0);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            shade = 2;
            break;
        case VoxelFace::BK:
            face_normal = vector3f_t(0.0f, 0.0f, -1.0f);
            offsets[0] = local_pos_t(0, 0, 1);
            offsets[1] = local_pos_t(1, 0, 1);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(0, 1, 1);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            shade = 2;
            break;
        case VoxelFace::UP:
            face_normal = vector3f_t(0.0f, 1.0f, 0.0f);
            offsets[0] = local_pos_t(0, 1, 0);
            offsets[1] = local_pos_t(0, 1, 1);
            offsets[2] = local_pos_t(1, 1, 1);
            offsets[3] = local_pos_t(1, 1, 0);
            texcoords[0] = vector2f_t(1.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 1.0f);
            texcoords[2] = vector2f_t(0.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 0.0f);
            shade = 3;
            break;
        case VoxelFace::DN:
            face_normal = vector3f_t(0.0f, -1.0f, 0.0f);
            offsets[0] = local_pos_t(0, 0, 0);
            offsets[1] = local_pos_t(1, 0, 0);
            offsets[2] = local_pos_t(1, 0, 1);
            offsets[3] = local_pos_t(0, 0, 1);
            texcoords[0] = vector2f_t(0.0f, 0.0f);
            texcoords[1] = vector2f_t(1.0f, 0.0f);
            texcoords[2] = vector2f_t(1.0f, 1.0f);
            texcoords[3] = vector2f_t(0.0f, 1.0f);
            shade = 0;
            break;
        default:
            // oopsie
            return;
    }

    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[0]), texcoords[0] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[1]), texcoords[1] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[2]), texcoords[2] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
    builder.vertex(PackedVertex(vector3f_t(lpos + offsets[3]), texcoords[3] * atlas_entry->max_texcoord, shade, face_normal, atlas_entry->entry_index));
    builder.index(base + 0);
    builder.index(base + 1);
    builder.index(base + 2);
    builder.index(base + 2);
    builder.index(base + 3);
    builder.index(base + 0);
    base += 4;
}

static inline bool isFaceTransparent(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t compare, VoxelFace face)
{
    const voxel_pos_t &vpos = world::getVoxelPosition(cpos, lpos);
    if(const ClientChunk *chunk = globals::chunks.find(world::getChunkPosition(vpos))) {
        const voxel_t voxel = chunk->data.at(world::getVoxelIndex(world::getLocalPosition(vpos)));
        if(compare != voxel) {
            if(const VoxelDefEntry *def = globals::voxels.find(voxel)) {
                const auto def_face = def->faces.find(face);
                if(def_face != def->faces.cend())
                    return def_face->second.transparent;
                return true;
            }

            return true;
        }

        // Voxels with the same type that are contiguous
        // should have no faces between each other.
        return false;
    }

    return true;
}
#include <sstream>
static void doNaiveMeshing(TerrainMeshBuilder &builder, const chunk_pos_t &cpos)
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

                        if(!isFaceTransparent(cpos, neighbour_lpos, voxel, neighbour_face))
                            continue;
                        pushCubeFace(builder, entry, lpos, def_face.first, base);
                    }
                }
            }
        }
    }
}

void terrain_mesher::update()
{
    const auto cube_group = globals::registry.group<StaticChunkMeshNeedsUpdateComponent<VoxelType::STATIC_CUBE>>(entt::get<ChunkComponent>);
    for(const auto [entity, chunk] : cube_group.each()) {
        StaticChunkMeshComponent &mesh = globals::registry.get_or_emplace<StaticChunkMeshComponent>(entity);

        TerrainMeshBuilder builder;
        doNaiveMeshing(builder, chunk.cpos);

        if(!builder.empty()) {
            if(!mesh.cube) {
                mesh.cube = std::make_unique<Mesh>();
                mesh.cube->ibo.create();
                mesh.cube->vbo.create();
                mesh.cube->vao.create();
                mesh.cube->cmd.create();

                mesh.cube->vao.setIndexBuffer(mesh.cube->ibo);
                mesh.cube->vao.setVertexBuffer(0, mesh.cube->vbo, sizeof(PackedVertex));

                mesh.cube->vao.enableAttribute(0, true);
                mesh.cube->vao.enableAttribute(1, true);
                mesh.cube->vao.enableAttribute(2, true);

                mesh.cube->vao.setAttributeFormat(0, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_0), false);
                mesh.cube->vao.setAttributeFormat(1, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_1), false);
                mesh.cube->vao.setAttributeFormat(2, GL_UNSIGNED_INT, 1, offsetof(PackedVertex, pack_2), false);

                mesh.cube->vao.setAttributeBinding(0, 0);
                mesh.cube->vao.setAttributeBinding(1, 0);
                mesh.cube->vao.setAttributeBinding(2, 0);
            }

            mesh.cube->ibo.resize(builder.isize(), builder.idata(), GL_STATIC_DRAW);
            mesh.cube->vbo.resize(builder.vsize(), builder.vdata(), GL_STATIC_DRAW);
            mesh.cube->cmd.set(GL_TRIANGLES, GL_UNSIGNED_SHORT, builder.icount(), 1, 0, 0, 0);
        }

        globals::registry.remove<StaticChunkMeshNeedsUpdateComponent<VoxelType::STATIC_CUBE>>(entity);

        spdlog::info("{}", cube_group.size());
        break;
    }
}
