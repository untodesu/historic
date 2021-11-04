/*
 * spdlog.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <spdlog/spdlog.h>

namespace util
{
template<typename T, typename... AT>
inline spdlog::sink_ptr createSinkPattern(const std::string &pattern, AT &&... ctor_args)
{
    spdlog::sink_ptr sink = std::make_shared<T>(std::forward<AT>(ctor_args)...);
    sink->set_pattern(pattern);
    return sink;
}

template<typename T, typename... AT>
inline spdlog::sink_ptr createSink(AT &&... ctor_args)
{
    // This is just a nice "default" pattern
    // that would be enough for mostly any sink.
    return util::createSinkPattern<T>("[%H:%M:%S] %^[%L]%$ %v", std::forward<AT>(ctor_args)...);
}
} // namespace util
