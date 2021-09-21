/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/packing.hpp>
#include <math/types.hpp>
#include <math/util.hpp>

struct BasicVertex final {
    float3 position;
    float2 texcoord;
    BasicVertex(const float3 &position, const float2 &texcoord);
};

struct VoxelVertex final {
    constexpr static const float PACK_EPSILON = 1.0f / 16.0f;
    uint32_t pack[2];
    VoxelVertex() = default;
    VoxelVertex(const float3 &position, const float2 &texcoord, uint16_t atlas_id);
};

inline BasicVertex::BasicVertex(const float3 &position, const float2 &texcoord)
    : position(position), texcoord(texcoord)
{

}

inline VoxelVertex::VoxelVertex(const float3 &position, const float2 &texcoord, uint16_t atlas_id)
    : pack { 0, 0 }
{
    pack[0] |= glm::packUnorm4x8(float4(position.x, position.y, position.z, 0.0f) * PACK_EPSILON);
    pack[1] |= glm::packUnorm4x8(float4(texcoord.x, texcoord.y, 0.0f, 0.0f) * PACK_EPSILON) & 0x0000FFFF;
    pack[1] |= static_cast<uint32_t>(atlas_id) << 16;
}
