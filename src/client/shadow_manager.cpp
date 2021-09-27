/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/shadow_manager.hpp>
#include <math/const.hpp>
#include <spdlog/spdlog.h>
#include <client/input.hpp>

static floatquat light_orientation = FLOATQUAT_IDENTITY;
static float3 light_direction = FLOAT3_ZERO;
static float3 light_color = FLOAT3_IDENTITY;
static float2 polygon_offset = FLOAT2_ZERO;
static ShadowMap shadowmap;

static inline void calculateDirection()
{
    // The light_orientation quaternion "looks"
    // at the "sun" thus the direction is
    // inverse from FLOAT3_FORWARD applied to it.
    light_direction = light_orientation * FLOAT3_FORWARD * -1.0f;
}

void shadow_manager::init(int width, int height)
{
    light_orientation = FLOATQUAT_IDENTITY;
    light_color = FLOAT3_IDENTITY;
    polygon_offset = FLOAT2_ZERO;
    shadowmap.init(width, height, gl::PixelFormat::D32_FLOAT);
    calculateDirection();
}

void shadow_manager::shutdown()
{
    shadowmap.shutdown();
}

void shadow_manager::rotateLight(float angle, const float3 &axis)
{
    light_orientation = glm::rotate(light_orientation, angle, axis);
    calculateDirection();
}

void shadow_manager::setLightOrientation(const floatquat &orientation)
{
    light_orientation = orientation;
    calculateDirection();
}

void shadow_manager::setLightColor(const float3 &color)
{
    light_color = color;
}

void shadow_manager::setPolygonOffset(const float2 &offset)
{
    polygon_offset = offset;
}

const floatquat &shadow_manager::getLightOrientation()
{
    return light_orientation;
}

const float3 &shadow_manager::getLightDirection()
{
    return light_direction;
}

const float3 &shadow_manager::getLightColor()
{
    return light_color;
}

const float2 &shadow_manager::getPolygonOffset()
{
    return polygon_offset;
}

const ShadowMap &shadow_manager::getShadowMap()
{
    return shadowmap;
}

const float4x4 shadow_manager::getProjView(const float3 &position)
{
    float4x4 projview = FLOAT4X4_IDENTITY;
    projview *= glm::ortho(-128.0f, 128.0f, -128.0f, 128.0f, -512.0f, 512.0f);
    projview *= glm::lookAt(position - light_direction, position, FLOAT3_UP);
    return projview;
}
