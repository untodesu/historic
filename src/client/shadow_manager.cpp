/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/shadow_manager.hpp>
#include <spdlog/spdlog.h>

static float3 shadow_angles = FLOAT3_ZERO;
static float3 shadow_light_direction = -FLOAT3_UP;
static float3 shadow_light_color = FLOAT3_IDENTITY;
static float2 shadow_size = FLOAT2_ZERO;
static ShadowMap shadow_shadowmap;

static inline void recalculateDirection()
{
    const float p = shadow_angles.x;
    const float y = shadow_angles.y;
    const float r = shadow_angles.z;
    shadow_light_direction.x = -glm::cos(r) * glm::sin(p) * glm::sin(y ) - glm::sin(r) * glm::cos(y );
    shadow_light_direction.y = -glm::sin(r) * glm::sin(p) * glm::sin(y ) - glm::cos(r) * glm::cos(y );
    shadow_light_direction.z =  glm::cos(p) * glm::sin(y);
    spdlog::info("{} {} {}", shadow_light_direction.x, shadow_light_direction.y, shadow_light_direction.z);
}

void shadow_manager::init(int size)
{
    shadow_angles = FLOAT3_ZERO;
    shadow_light_color = FLOAT3_IDENTITY;
    shadow_size = float2(size, size);
    recalculateDirection();
    shadow_shadowmap.init(size, size, gl::PixelFormat::D32_FLOAT);
}

void shadow_manager::shutdown()
{
    shadow_shadowmap.shutdown();
}

void shadow_manager::setAngles(const float3 &angles)
{
    shadow_angles = angles;
    recalculateDirection();
}

void shadow_manager::setLightColor(const float3 &color)
{
    shadow_light_color = color;
}

const float3 &shadow_manager::angles()
{
    return shadow_angles;
}

const float3 &shadow_manager::lightDirection()
{
    return shadow_light_direction;
}

const float3 &shadow_manager::lightColor()
{
    return shadow_light_color;
}

const float2 &shadow_manager::size()
{
    return shadow_size;
}

const ShadowMap &shadow_manager::shadowmap()
{
    return shadow_shadowmap;
}

const float4x4 shadow_manager::matrix(const float3 &position)
{
    float4x4 projview = FLOAT4X4_IDENTITY;
    projview *= glm::ortho(-16.0f, 16.0f, -16.0f, 16.0f, -512.0f, 512.0f);
    projview *= glm::lookAt(position - shadow_light_direction, position, FLOAT3_UP);
    return projview;
}
