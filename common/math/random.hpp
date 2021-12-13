/*
 * random.hpp
 * Author: Kirill GPRB
 * Created: Mon Dec 13 2021 14:43:04
 */
#pragma once
#include <common/math/math.hpp>
#include <random>
#include <string>

namespace math
{
template<typename T>
static inline const std::string randomString(T &rng, size_t n)
{
    constexpr static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    constexpr static const size_t nv = math::arraySize(alphanum);
    std::uniform_int_distribution<size_t> distrib(0, nv);
    std::string str = std::string(n, static_cast<char>(0));
    for(size_t i = 0; i < n; i++)
        str[i] = alphanum[distrib(rng)];
    return str;
}
} // namespace math
