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

struct PackedVertex final {
    uint32_t pack[3];
    PackedVertex() = default;
    PackedVertex(const float3 &position, const float3 &normal, const float2 &texcoord, uint16_t atlas_id);
};

inline BasicVertex::BasicVertex(const float3 &position, const float2 &texcoord)
    : position(position), texcoord(texcoord)
{

}

inline PackedVertex::PackedVertex(const float3 &position, const float3 &normal, const float2 &texcoord, uint16_t atlas_id)
    : pack { 0, 0 }
{
    pack[0] |= glm::packUnorm4x8(float4(position.x, position.y, position.z, 0.0f) / 16.0f);
    pack[1] |= glm::packSnorm4x8(float4(normal.x, normal.y, normal.z, 0.0f));
    pack[2] |= glm::packUnorm4x8(float4(texcoord.x, texcoord.y, 0.0f, 0.0f) / 16.0f) & 0x0000FFFF;
    pack[2] |= static_cast<uint32_t>(atlas_id) << 16;
}
