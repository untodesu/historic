/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/shadowmap.hpp>

// Let me speak to your shadow manager!
namespace shadow_manager
{
void init(int size = 2048);
void shutdown();
void setAngles(const float3 &angles);
void setLightColor(const float3 &color);
const float3 &angles();
const float3 &lightDirection();
const float3 &lightColor();
const float2 &size();
const ShadowMap &shadowmap();
const float4x4 matrix(const float3 &position);
} // namespace shadow_manager
