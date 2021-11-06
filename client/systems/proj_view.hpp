/*
 * proj_view.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/frustum.hpp>

namespace proj_view
{
void init();
void update();
const float2 &angles();
const float3 &position();
const float4x4 &matrix();
const float4x4 &matrixShadow();
const Frustum &frustum();
const Frustum &frustumShadow();
} // namespace proj_view
