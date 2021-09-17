/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/util.hpp>
#include <client/world.hpp>
#include <shared/comp/chunk.hpp>

voxel_t client_util::getVoxel(const voxelpos_t &vp)
{
    const chunkpos_t &cp = toChunkPos(vp);
    const voxelidx_t idx = toVoxelIdx(toLocalPos(vp));
    const auto view = client_world::registry().view<ChunkComponent>();
    for(const auto [entity, chunk] : view.each()) {
        if(chunk.position != cp)
            continue;
        return chunk.data[idx];
    }

    return NULL_VOXEL;
}

voxel_t client_util::getVoxel(const chunkpos_t &cp, const localpos_t &lp)
{
    // localpos_t can have negative values and 
    // values that are way over CHUNK_SIZE. This allows
    // getVoxel() to return voxel data past the chunk specified
    return client_util::getVoxel(toVoxelPos(cp, lp));
}
