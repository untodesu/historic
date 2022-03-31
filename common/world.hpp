/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <array>
#include <common/math/constexpr.hpp>
#include <functional>

constexpr static const size_t CHUNK_SIZE = 16;
constexpr static const size_t CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_SIZE_LOG2 = math::log2(CHUNK_SIZE);

using chunk_pos_t = vector3i32_t;
using local_pos_t = vector3i16_t;
using voxel_pos_t = vector3i64_t;
using voxel_idx_t = size_t;

// As of now voxel IDs are 8-bit values,
// not great not terrible, but in future
// with planned mod support, this won't be enough.
using voxel_t = uint8_t;
using chunk_t = std::array<voxel_t, CHUNK_VOLUME>;

// READABILITY: instead of 0 use NULL_VOXEL
constexpr static const voxel_t NULL_VOXEL = 0;

template<>
struct std::hash<chunk_pos_t> final {
    size_t operator()(const chunk_pos_t &cpos) const
    {
        return (cpos.x * 73856093) ^ (cpos.y * 19349663) ^ (cpos.z * 83492791);
    }
};

namespace world
{
constexpr static inline const chunk_pos_t getChunkPosition(const vector3f_t &wpos)
{
    return chunk_pos_t {
        static_cast<chunk_pos_t::value_type>(wpos.x) >> CHUNK_SIZE_LOG2,
        static_cast<chunk_pos_t::value_type>(wpos.y) >> CHUNK_SIZE_LOG2,
        static_cast<chunk_pos_t::value_type>(wpos.z) >> CHUNK_SIZE_LOG2
    };
}

constexpr static inline const chunk_pos_t getChunkPosition(const voxel_pos_t &vpos)
{
    return chunk_pos_t {
        static_cast<chunk_pos_t::value_type>(vpos.x >> CHUNK_SIZE_LOG2),
        static_cast<chunk_pos_t::value_type>(vpos.y >> CHUNK_SIZE_LOG2),
        static_cast<chunk_pos_t::value_type>(vpos.z >> CHUNK_SIZE_LOG2)
    };
}

constexpr static inline const local_pos_t getLocalPosition(const voxel_pos_t &vpos)
{
    return local_pos_t {
        glm::abs(static_cast<local_pos_t::value_type>(vpos.x % CHUNK_SIZE)),
        glm::abs(static_cast<local_pos_t::value_type>(vpos.y % CHUNK_SIZE)),
        glm::abs(static_cast<local_pos_t::value_type>(vpos.z % CHUNK_SIZE))
    };
}

constexpr static inline const local_pos_t getLocalPosition(const voxel_idx_t &index)
{
    return local_pos_t {
        static_cast<local_pos_t::value_type>(index / CHUNK_AREA),
        static_cast<local_pos_t::value_type>(index % CHUNK_SIZE),
        static_cast<local_pos_t::value_type>((index / CHUNK_SIZE) % CHUNK_SIZE)
    };
}

constexpr static inline const voxel_pos_t getVoxelPosition(const vector3f_t &wpos)
{
    return voxel_pos_t(wpos.x, wpos.y, wpos.z);
}

constexpr static inline const voxel_pos_t getVoxelPosition(const chunk_pos_t &cpos, const local_pos_t &lpos)
{
    return voxel_pos_t {
        static_cast<voxel_pos_t::value_type>(lpos.x + (cpos.x << CHUNK_SIZE_LOG2)),
        static_cast<voxel_pos_t::value_type>(lpos.y + (cpos.y << CHUNK_SIZE_LOG2)),
        static_cast<voxel_pos_t::value_type>(lpos.z + (cpos.z << CHUNK_SIZE_LOG2))
    };
}

constexpr static inline const voxel_idx_t getVoxelIndex(const local_pos_t &lpos)
{
    return static_cast<voxel_idx_t>((lpos.x * CHUNK_SIZE + lpos.z) * CHUNK_SIZE + lpos.y);
}

constexpr static inline const vector3f_t getChunkWorldPosition(const chunk_pos_t &cpos)
{
    return vector3f_t {
        static_cast<float>(cpos.x << CHUNK_SIZE_LOG2),
        static_cast<float>(cpos.y << CHUNK_SIZE_LOG2),
        static_cast<float>(cpos.z << CHUNK_SIZE_LOG2),
    };
}
} // namespace world
