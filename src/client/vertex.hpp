/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/packing.hpp>
#include <math/const.hpp>

struct Vertex final {
    float3 position;
    float3 normal;
    float2 texcoord;
    uint32_t atlas_id;
    Vertex();
    Vertex(const float3 &position, const float3 &normal, const float2 &texcoord, uint32_t atlas_id);
};

inline Vertex::Vertex()
    : position(FLOAT3_ZERO), normal(FLOAT3_ZERO), texcoord(FLOAT2_ZERO), atlas_id(0)
{
    
}

inline Vertex::Vertex(const float3 &position, const float3 &normal, const float2 &texcoord, uint32_t atlas_id)
    : position(position), normal(normal), texcoord(texcoord), atlas_id(atlas_id)
{

}
