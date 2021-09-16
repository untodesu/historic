/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/util/chunks.hpp>
#include <client/client_world.hpp>
#include <shared/comp/chunk.hpp>

static bool getVoxelAt(const chunkpos_t &cp, const localpos_t &lp, voxel_t &voxel)
{
    const auto view = client_world::registry().view<ChunkComponent>();
    for(const auto [entity, chunk] : view.each()) {
        if(chunk.position != cp)
            continue;
        voxel = chunk.data[toVoxelIdx(lp)];
        return true;
    }

    return false;
}

bool util::isVoxelAt(const chunkpos_t &cp, const localpos_t &lp)
{
    voxel_t voxel;
    if(getVoxelAt(cp, lp, voxel))
        return voxel != NULL_VOXEL;
    return false;
}

bool util::isVoxelAt(const chunkpos_t &cp, const localpos_t &lp, voxel_t compare)
{
    voxel_t voxel;
    if(getVoxelAt(cp, lp, voxel))
        return voxel == compare;
    return false;
}
