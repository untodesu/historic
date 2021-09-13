/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math_types.hpp>

constexpr static const float2_t FLOAT2_ZERO = { 0.0f, 0.0f };
constexpr static const float3_t FLOAT3_ZERO = { 0.0f, 0.0f, 0.0f };
constexpr static const float4_t FLOAT4_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };

constexpr static const float2_t FLOAT2_IDENTITY = { 1.0f, 1.0f };
constexpr static const float3_t FLOAT3_IDENTITY = { 1.0f, 1.0f, 1.0f };
constexpr static const float4_t FLOAT4_IDENTITY = { 1.0f, 1.0f, 1.0f, 1.0f };
constexpr static const float4x4_t FLOAT4X4_IDENTITY = glm::identity<float4x4_t>();
constexpr static const floatquat_t FLOATQUAT_IDENTITY = glm::identity<floatquat_t>();

constexpr static const float3_t FLOAT3_FORWARD = { 0.0f, 0.0f, -1.0f };
constexpr static const float3_t FLOAT3_RIGHT = { 1.0f, 0.0f, 0.0f };
constexpr static const float3_t FLOAT3_UP = { 0.0f, 1.0f, 0.0f };

constexpr static const float ANG_89NEG = glm::radians(89.0f) * -1.0f;
constexpr static const float ANG_89POS = glm::radians(89.0f);
