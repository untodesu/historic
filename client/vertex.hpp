/*
 * vertex.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
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
