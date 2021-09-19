/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <array>
#include <math/types.hpp>
#include <math/util.hpp>
#include <string>
#include <utility>
#include <vector>

constexpr static const size_t CHUNK_SIZE = 32;
constexpr static const size_t CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_BITSHIFT = math::log2(CHUNK_SIZE);

using chunkpos_t = glm::vec<3, int32_t, glm::packed_highp>;
using voxelpos_t = glm::vec<3, int64_t, glm::packed_highp>;
using localpos_t = glm::vec<3, int16_t, glm::packed_highp>;
using voxelidx_t = size_t;
using voxel_t = uint8_t;
using voxel_array_t = std::array<voxel_t, CHUNK_VOLUME>;

constexpr static const voxel_t NULL_VOXEL = 0x00;
constexpr static const size_t MAX_VOXELS = static_cast<size_t>(std::numeric_limits<voxel_t>::max()) + 1;

template<>
struct std::hash<chunkpos_t> final {
    size_t operator()(const chunkpos_t &cp) const
    {
        return (cp.x * 73856093) ^ (cp.y * 19349663) ^ (cp.z * 83492791);
    }
};

constexpr static inline const chunkpos_t toChunkPos(const float3_t &wp)
{
    return chunkpos_t(static_cast<int32_t>(wp.x) >> CHUNK_BITSHIFT, static_cast<int32_t>(wp.y) >> CHUNK_BITSHIFT, static_cast<int32_t>(wp.z) >> CHUNK_BITSHIFT);
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

constexpr static inline const bool isLocalPosInRange(const localpos_t &lp)
{
    if(lp.x < 0 || lp.y < 0 || lp.z < 0)
        return false;
    if(lp.x >= CHUNK_SIZE || lp.y >= CHUNK_SIZE || lp.z >= CHUNK_SIZE)
        return false;
    return true;
}
