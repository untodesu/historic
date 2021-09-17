/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <shared/voxel_def.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

static std::unordered_map<voxel_t, VoxelInfo> def;
static std::vector<voxel_t> def_list;

void voxel_def::add(voxel_t voxel, const VoxelInfo &info)
{
    // voxel_face_t is a bitmask. With that in mind
    // we need to convert the separate face textures
    // into a single big bitmask to avoid generating
    // a fuck-ton of meshes for a fuck-ton of faces.
    std::vector<VoxelFaceInfo> patched_faces;
    for(const VoxelFaceInfo &it_i : info.faces) {
        bool skip = false;

        // Firstly determine if we already have such
        // face definition in the patched vector. If such
        // thing is already present, we skip this face.
        for(const VoxelFaceInfo &it_j : patched_faces) {
            if(it_j.mask == it_i.mask) {
                skip = true;
                break;
            }
        }

        if(!skip) {
            // Now we go through the patched faces to see
            // if any face has the same texture as unpached one.
            // If we find one, we attach ourselves to it.
            for(VoxelFaceInfo &it_j : patched_faces) {
                if(it_j.texture == it_i.texture) {
                    it_j.mask |= it_i.mask;
                    skip = true;
                    break;
                }
            }

            // If there's no suitable place for the patched
            // face yet, we create a new one.
            if(!skip) {
                patched_faces.push_back(it_i);
            }
        }
    }

    VoxelInfo patched_info = {};
    patched_info.type = info.type;
    patched_info.transparency = info.transparency;
    patched_info.faces = patched_faces;

    if(def.find(voxel) != def.cend())
        spdlog::warn("Overriding VoxelInfo for {}", voxel);
    else
        def_list.push_back(voxel);
    def[voxel] = patched_info;
}

bool voxel_def::exists(voxel_t voxel)
{
    return def.find(voxel) != def.cend();
}

const VoxelInfo *voxel_def::get(voxel_t voxel)
{
    const auto it = def.find(voxel);
    if(it != def.cend())
        return &it->second;
    return nullptr;
}

size_t voxel_def::listSize()
{
    return def_list.size();
}

const voxel_t *voxel_def::list()
{
    return def_list.data();
}
