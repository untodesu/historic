/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/pack.hpp>
#include <math/vertex.hpp>

struct PackedVertex final {
    constexpr static const float MAX_POSITION = 128.0f;
    constexpr static const float MAX_TEXCOORD = 128.0f;
    uint32_t position[2];
    uint32_t texcoord;
    PackedVertex(const Vertex &vtx);
    PackedVertex(const float3_t &position, const float2_t &texcoord);
};

inline PackedVertex::PackedVertex(const Vertex &vtx)
    : PackedVertex(vtx.position, vtx.texcoord)
{
}

inline PackedVertex::PackedVertex(const float3_t &position, const float2_t &texcoord)
{
    this->position[0] = math::packUnorm2x16(position.x / MAX_POSITION, position.y / MAX_POSITION);
    this->position[1] = math::packUnorm2x16(position.z / MAX_POSITION, 0.0f);
    this->texcoord = math::packUnorm2x16(texcoord.x / MAX_TEXCOORD, texcoord.y / MAX_TEXCOORD);
}
