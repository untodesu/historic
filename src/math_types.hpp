/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stddef.h>
#include <stdint.h>

using float2 = glm::vec<2, float, glm::packed_highp>;
using float3 = glm::vec<3, float, glm::packed_highp>;
using float4 = glm::vec<4, float, glm::packed_highp>;
using float4x4 = glm::mat<4, 4, float, glm::packed_highp>;
using floatquat = glm::qua<float, glm::packed_highp>;

static constexpr const float2 FLOAT2_ZERO = { 0.0f, 0.0f };
static constexpr const float3 FLOAT3_ZERO = { 0.0f, 0.0f, 0.0f };
static constexpr const float4 FLOAT4_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };

static constexpr const float2 FLOAT2_IDENTITY = { 1.0f, 1.0f };
static constexpr const float3 FLOAT3_IDENTITY = { 1.0f, 1.0f, 1.0f };
static constexpr const float4 FLOAT4_IDENTITY = { 1.0f, 1.0f, 1.0f, 1.0f };
static constexpr const float4x4 FLOAT4X4_IDENTITY = glm::identity<float4x4>();
static constexpr const floatquat FLOATQUAT_IDENTITY = glm::identity<floatquat>();

static constexpr const float3 FLOAT3_FORWARD = { 0.0f, 0.0f, -1.0f };
static constexpr const float3 FLOAT3_RIGHT = { 1.0f, 0.0f, 0.0f };
static constexpr const float3 FLOAT3_UP = { 0.0f, 1.0f, 0.0f };
