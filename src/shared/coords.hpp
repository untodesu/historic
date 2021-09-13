/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/world_defs.hpp>

constexpr static inline const chunkpos_t toChunkPos(const float3_t &wp)
{
    return chunkpos_t(static_cast<int64_t>(wp.x) >> CHUNK_BITSHIFT, static_cast<int64_t>(wp.y) >> CHUNK_BITSHIFT, static_cast<int64_t>(wp.z) >> CHUNK_BITSHIFT);
}

constexpr static inline const chunkpos_t toChunkPos(const voxelpos_t &vp)
{
    return chunkpos_t(vp.x >> CHUNK_BITSHIFT, vp.y >> CHUNK_BITSHIFT, vp.z >> CHUNK_BITSHIFT);
}

constexpr static inline const voxelpos_t toVoxelPos(const float3_t &wp)
{
    return voxelpos_t(wp.x, wp.y, wp.z);
}

constexpr static inline const voxelpos_t toVoxelPos(const chunkpos_t &cp, const localpos_t &lp)
{
    return voxelpos_t((cp.x << CHUNK_BITSHIFT) + lp.x, (cp.y << CHUNK_BITSHIFT) + lp.y, (cp.z << CHUNK_BITSHIFT) + lp.z);
}

constexpr static inline const localpos_t toLocalPos(const voxelpos_t &vp)
{
    return glm::abs(localpos_t(vp.x % CHUNK_SIZE, vp.y % CHUNK_SIZE, vp.z % CHUNK_SIZE));
}

constexpr static inline const localpos_t toLocalPos(const voxelidx_t &vi)
{
    return localpos_t(vi / CHUNK_AREA, vi % CHUNK_SIZE, (vi / CHUNK_SIZE) % CHUNK_SIZE);
}

constexpr static inline const voxelidx_t toVoxelIdx(const localpos_t &lp)
{
    return static_cast<voxelidx_t>((lp.x * CHUNK_SIZE + lp.z) * CHUNK_SIZE + lp.y);
}

constexpr static inline const float3_t toWorldPos(const chunkpos_t &cp)
{
    return float3_t(cp.x << CHUNK_BITSHIFT, cp.y << CHUNK_BITSHIFT, cp.z << CHUNK_BITSHIFT);
}
