/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/packing.hpp>
#include <common/math/const.hpp>

struct Vertex final {
    float3 position;
    float3 normal;
    float2 texcoord;
    uint32_t atlas_id;
};

struct QuadVertex2D final {
    float2 position;
    float2 texcoord;
};
