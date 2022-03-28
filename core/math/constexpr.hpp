/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <core/types.hpp>
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
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_floating_point_v<F>);
    const T ival = static_cast<T>(x);
    if(ival == static_cast<F>(ival))
        return ival;
    return ival + ((x > 0) ? 1 : 0);
}

template<typename T>
constexpr static inline const T pow2(const T x)
{
    T value = 1;
    while(value < x)
        value *= 2;
    return value;
}

template<typename T>
constexpr static inline const T min(const T x, const T y)
{
    if(x > y)
        return y;
    return x;
}

template<typename T>
constexpr static inline const T max(const T x, const T y)
{
    if(x < y)
        return y;
    return x;
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

template<typename T, size_t L>
constexpr static inline const size_t arraySize(T(&)[L])
{
    return L;
}

constexpr static inline const bool isInBB(const vector3f_t &p, const vector3f_t &a, const vector3f_t &b)
{
    return p.x >= a.x && p.y >= a.y && p.z >= a.z && p.x <= b.x && p.y <= b.y && p.z <= b.z;
}

template<typename T>
constexpr static inline void vecToArray(const T &vec, typename T::value_type array[T::length()])
{
    for(typename T::length_type i = 0; i < T::length(); i++)
        array[i] = vec[i];
}

template<typename T>
constexpr static inline const T arrayToVec(const typename T::value_type array[T::length()])
{
    T vec;
    for(typename T::length_type i = 0; i < T::length(); i++)
        vec[i] = array[i];
    return vec;
}
} // namespace math
