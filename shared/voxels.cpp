/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>

VoxelDef::VoxelDef()
    : def(), checksum(0)
{
}

void VoxelDef::clear()
{
    checksum = 0;
    def.clear();
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

    for(const VoxelFaceInfo &face : info.faces)
        checksum++;
    def[voxel] = info;
    return true;
}

const VoxelInfo *VoxelDef::tryGet(voxel_t voxel) const
{
    const auto it = def.find(voxel);
    if(it != def.cend())
        return &it->second;
    return nullptr;
}
