/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math_types.hpp>

struct Vertex {
    float3_t position;
    float2_t texcoord;
};

struct PackedVertex {
    constexpr PackedVertex(const float3_t &position, const float2_t &texcoord);
    uint16_t position;
    uint16_t texcoord;
};

constexpr inline PackedVertex::PackedVertex(const float3_t &position, const float2_t &texcoord)
{
    this->position = ((static_cast<uint16_t>(position.x) & 0x1F) << 11) | ((static_cast<uint16_t>(position.y) & 0x1F) << 6) | ((static_cast<uint16_t>(position.z) & 0x1F) << 1);
    this->position = (static_cast<uint16_t>(texcoord.x) << 8) | static_cast<uint16_t>(texcoord.y);
}
