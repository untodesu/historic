/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/trigonometric.hpp>
#include <type_traits>

namespace math
{
template<typename T>
constexpr static inline const T log2(const T x)
{
    if(x < 2)
        return 0;
    return math::log2<T>((x + 1) >> 1) + 1;
}

template<typename T, typename F>
constexpr static inline const T ceil(const F x)
{
    static_assert(std::is_floating_point<F>::value);
    T ival = static_cast<T>(x);
    if(ival == static_cast<F>(ival))
        return ival;
    return ival + ((x > 0) ? 1 : 0);
}

template<typename T>
constexpr static inline const T clamp(const T x, const T min, const T max)
{
    if(x < min)
        return min;
    if(x > max)
        return max;
    return x;
}

template<typename F>
static inline const F fixAngle180(const F angle)
{
    static_assert(std::is_floating_point<F>::value);
    constexpr const F na_180 = glm::radians<F>(180.0f);
    constexpr const F na_360 = glm::radians<F>(360.0f);
    const F norm = std::fmod(angle + na_180, na_360);
    return ((norm < 0.0f) ? (norm + na_360) : norm) - na_180;
}

template<typename F, glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, F, Q> fixAngle180(const glm::vec<L, F, Q> &angles)
{
    glm::vec<L, F, Q> result;
    for(glm::length_t i = 0; i < L; i++)
        result[i] = math::fixAngle180<F>(angles[i]);
    return result;
}

template<typename F>
static inline const F fixAngle360(const F angle)
{
    static_assert(std::is_floating_point<F>::value);
    constexpr const F na_360 = glm::radians<F>(360.0f);
    const F norm = std::fmod(angle, na_360);
    return (norm < 0.0f) ? (norm + na_360) : norm;
}

template<typename F, glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, F, Q> fixAngle360(const glm::vec<L, F, Q> &angles)
{
    glm::vec<L, F, Q> result;
    for(glm::length_t i = 0; i < L; i++)
        result[i] = math::fixAngle360<F>(angles[i]);
    return result;
}

template<typename T, size_t L>
constexpr static inline const size_t arraySize(T(&)[L])
{
    return L;
}

constexpr static inline const bool isInBB(const float3_t &p, const float3_t &a, const float3_t &b)
{
    return p.x >= a.x && p.y >= a.y && p.z >= a.z && p.x <= b.x && p.y <= b.y && p.z <= b.z;
}
} // namespace math
