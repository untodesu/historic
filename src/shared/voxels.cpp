/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>

VoxelDef::VoxelDef()
    : def()
{
}

bool VoxelDef::set(voxel_t voxel, const VoxelInfo &info)
{
    if(voxel == NULL_VOXEL) {
        spdlog::error("VoxelDef: NULL_VOXEL is reserved!");
        return false;
    }

    if(def.find(voxel) != def.cend()) {
        spdlog::error("VoxelDef: voxel {} is already present!", voxel);
        return true;
    }

    // Voxel face is a bit mask. With that in mind
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

    def[voxel] = patched_info;
    return true;
}

const VoxelInfo *VoxelDef::tryGet(voxel_t voxel) const
{
    const auto it = def.find(voxel);
    if(it != def.cend())
        return &it->second;
    return nullptr;
}
