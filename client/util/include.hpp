/*
 * include.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>

namespace util
{
bool include(const stdfs::path &path, std::string &out, const std::string &comment = "//");
} // namespace util
