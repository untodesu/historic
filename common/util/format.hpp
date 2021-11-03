/*
 * format.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <string>
#include <utility>
#include <cstdio>

namespace util
{
template<typename... T>
static inline const std::string format(const std::string &fmt, T &&... args)
{
    int size = std::snprintf(nullptr, 0, fmt.c_str(), std::forward<T>(args)...);
    if(size <= 0)
        return fmt;
    std::string buffer(static_cast<size_t>(size) + 1, 0);
    std::snprintf(buffer.data(), buffer.size(), fmt.c_str(), std::forward<T>(args)...);
    return buffer;
}
} // namespace util
