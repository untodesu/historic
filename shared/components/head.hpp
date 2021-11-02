/*
 * head.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/const.hpp>

struct HeadComponent final {
    float2 angles { FLOAT2_ZERO };
    float3 offset { FLOAT3_ZERO };
};
