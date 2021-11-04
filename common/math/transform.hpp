/*
 * transform.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>

namespace math
{
static inline const float3 worldToDevice(const float4x4 &wts, const float3 &wp)
{
    const float4 clip = wts * float4(wp, 1.0f);
    return float3(clip.x, clip.y, clip.z) / clip.w;
}

static inline const float2 deviceToScreen(const float3 &ndc, const float2 &ss)
{
    return (float2(ndc.x, -ndc.y) + 1.0f) * 0.5f * ss;
}
} // namespace math
