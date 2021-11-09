/*
 * shadow_manager.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>
#include <client/render/shadowmap.hpp>
#include <shared/script/cvar_numeric.hpp>

extern CVarInt r_shadowmapres;
extern CVarShort r_shadows;

// Let me speak to your shadow manager!
namespace shadow_manager
{
void preInit();
void init();
void shutdown();
void rotateLight(float angle, const float3 &axis);
void setLightOrientation(const floatquat &orientation);
void setLightColor(const float3 &color);
void setPolygonOffset(const float2 &offset);
const floatquat &getLightOrientation();
const float3 &getLightDirection();
const float3 &getLightColor();
const float2 &getPolygonOffset();
const ShadowMap &getShadowMap();
const float4x4 getProjView(const float3 &position);
} // namespace shadow_manager
