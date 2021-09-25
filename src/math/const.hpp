/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/types.hpp>

constexpr static const float2 FLOAT2_ZERO = { 0.0f, 0.0f };
constexpr static const float3 FLOAT3_ZERO = { 0.0f, 0.0f, 0.0f };
constexpr static const float4 FLOAT4_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };
constexpr static const hash_t HASH_ZERO = { 0 };

constexpr static const float2 FLOAT2_IDENTITY = { 1.0f, 1.0f };
constexpr static const float3 FLOAT3_IDENTITY = { 1.0f, 1.0f, 1.0f };
constexpr static const float4 FLOAT4_IDENTITY = { 1.0f, 1.0f, 1.0f, 1.0f };
constexpr static const float4x4 FLOAT4X4_IDENTITY = glm::identity<float4x4>();
constexpr static const floatquat FLOATQUAT_IDENTITY = glm::identity<floatquat>();

constexpr static const float3 FLOAT3_FORWARD = { 0.0f, 0.0f, -1.0f };
constexpr static const float3 FLOAT3_RIGHT = { 1.0f, 0.0f, 0.0f };
constexpr static const float3 FLOAT3_UP = { 0.0f, 1.0f, 0.0f };

constexpr static const float ANGLE_0D = glm::radians(0.0f);
constexpr static const float ANGLE_89D = glm::radians(89.0f);
constexpr static const float ANGLE_90D = glm::radians(90.0f);
constexpr static const float ANGLE_180D = glm::radians(180.0f);
constexpr static const float ANGLE_360D = glm::radians(360.0f);
