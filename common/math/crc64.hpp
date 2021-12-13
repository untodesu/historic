/*
 * crc64.hpp
 * Author: Kirill GPRB
 * Created: Mon Dec 13 2021 14:11:23
 */
#pragma once
#include <common/math/types.hpp>
#include <string>

namespace math
{
uint64_t crc64(const void *s, size_t n);
uint64_t crc64(const std::string &s);
} // namespace math
