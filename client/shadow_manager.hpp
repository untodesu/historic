/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <client/render/shadowmap.hpp>

// Let me speak to your shadow manager!
namespace shadow_manager
{
void init(int width, int height);
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
