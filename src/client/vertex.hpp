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
    float3_t pos;
    float2_t uv;
    BasicVertex(const float3_t &pos, const float2_t &uv);
};

struct VoxelVertex final {
    constexpr static const float POS_EPSILON = 1.0f / 32.0f;
    uint32_t pack[2];
    VoxelVertex(const float3_t &pos, const float2_t &uv, uint16_t tex_id);
};

inline BasicVertex::BasicVertex(const float3_t &pos, const float2_t &uv)
    : pos(pos), uv(uv)
{

}

inline VoxelVertex::VoxelVertex(const float3_t &pos, const float2_t &uv, uint16_t tex_id)
{
    pack[0] = (0x00FFFFFF & glm::packUnorm4x8(float4_t(pos.x, pos.y, pos.z, 0.0f) * POS_EPSILON));
    pack[1] = (0x0000FFFF & glm::packUnorm4x8(float4_t(uv.x, uv.y, 0.0f, 0.0f))) | (static_cast<uint32_t>(tex_id) << 16);
}
