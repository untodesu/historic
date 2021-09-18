/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/packing.hpp>
#include <client/vertex.hpp>

class PackedVertex final {
public:
    PackedVertex(const Vertex &vtx);
    PackedVertex(const float3_t &pos, const float2_t &uv, uint32_t idx);

public:
    uint32_t pos_uv[2];
    uint32_t tex_id;
};

inline PackedVertex::PackedVertex(const Vertex &vtx)
    : PackedVertex(vtx.position, vtx.texcoord, vtx.atlas_id)
{

}

inline PackedVertex::PackedVertex(const float3_t &position, const float2_t &texcoord, uint32_t atlas_id)
    : pos_uv{ 0, 0 }, tex_id(0)
{
    const float3_t patch = position / 64.0f;
    pos_uv[0] |= glm::packUnorm2x16(float2_t(patch.x, patch.y));
    pos_uv[1] |= glm::packUnorm2x16(float2_t(patch.z, 0.0f)) & 0x0000FFFF;
    pos_uv[1] |= glm::packUnorm2x8(texcoord) << 16;
    tex_id = atlas_id;
}
