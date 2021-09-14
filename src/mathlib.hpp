/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stddef.h>
#include <stdint.h>

using float2_t = glm::vec<2, float, glm::packed_highp>;
using float3_t = glm::vec<3, float, glm::packed_highp>;
using float4_t = glm::vec<4, float, glm::packed_highp>;
using float4x4_t = glm::mat<4, 4, float, glm::packed_highp>;
using floatquat_t = glm::qua<float, glm::packed_highp>;

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

constexpr static const float ANG_89 = glm::radians(89.0f);
constexpr static const float ANG_180 = glm::radians(180.0f);
constexpr static const float ANG_360 = glm::radians(360.0f);

namespace math
{

template<typename T>
constexpr static inline const T log2floor(const T x)
{
    if(x <= 1)
        return 0;
    return math::log2floor<T>(x >> 1) + 1;
}

template<typename T>
constexpr static inline const T log2ceil(const T x)
{
    if(x <= 1)
        return 0;
    return math::log2ceil((x + 1) >> 1) + 1;
}

template<typename T>
constexpr static inline const T log2(const T x)
{
    return math::log2ceil<T>(x);
}

template<typename T, typename F = float>
constexpr static inline const T ceil(const F x)
{
    T ival = static_cast<T>(x);
    if(ival == static_cast<F>(ival))
        return ival;
    return ival + ((x > 0) ? 1 : 0);
}

template<typename T>
constexpr static inline const T clamp(const T val, const T min, const T max)
{
    if(val < min)
        return min;
    if(val > max)
        return max;
    return val;
}

static inline const float normalizeAngle180(const float angle)
{
    constexpr const float na_180 = glm::radians(180.0f);
    constexpr const float na_360 = glm::radians(360.0f);
    const float norm = std::fmod(angle + na_180, na_360);
    return ((norm < 0.0f) ? (norm + na_360) : norm) - na_180;
}

static inline const float normalizeAngle360(const float angle)
{
    constexpr const float na_360 = glm::radians(360.0f);
    const float norm = std::fmod(angle, na_360);
    return (norm < 0.0f) ? (norm + na_360) : norm;
}

template<typename T, glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, T, Q> normalizeAngle180V(const glm::vec<L, T, Q> &av)
{
    glm::vec<L, T, Q> norm;
    for(glm::length_t i = 0; i < L; i++)
        norm[i] = math::normalizeAngle180(av[i]);
    return norm;
}

template<typename T, glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, T, Q> normalizeAngle360V(const glm::vec<L, T, Q> &av)
{
    glm::vec<L, T, Q> norm;
    for(glm::length_t i = 0; i < L; i++)
        norm[i] = math::normalizeAngle360(av[i]);
    return norm;
}
} // namespace math
