/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <spdlog/spdlog.h>
#include <string_view>

namespace util
{
template<typename sink_type, typename... args_type>
inline spdlog::sink_ptr makeSink(const std::string_view pattern, args_type &&... args)
{
    spdlog::sink_ptr sink = std::make_shared<sink_type>(std::forward<args_type>(args)...);
    sink->set_pattern(std::string(pattern.begin(), pattern.end()));
    return sink;
}

template<typename sink_type, typename... args_type>
inline spdlog::sink_ptr makeSinkSimple(args_type &&... args)
{
    return util::makeSink<sink_type>("[%H:%M:%S] %^[%L]%$ %v", std::forward<args_type>(args)...);
}
} // namespace util
