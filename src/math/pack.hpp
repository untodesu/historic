/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <limits>
#include <math/util.hpp>

namespace math
{
constexpr static inline const uint32_t packUnorm2x16(const float a, const float b)
{
    constexpr const float pack_factor = static_cast<float>(std::numeric_limits<uint16_t>::max());
    uint32_t value = 0;
    value |= (static_cast<uint32_t>(math::clamp(a, 0.0f, 1.0f) * pack_factor) & 0xFFFF);
    value |= (static_cast<uint32_t>(math::clamp(b, 0.0f, 1.0f) * pack_factor) & 0xFFFF) << 16;
    return value;
}

constexpr static inline const uint32_t packSnorm2x16(const float a, const float b)
{
    constexpr const float pack_factor = static_cast<float>(std::numeric_limits<int16_t>::max());
    uint32_t value = 0;
    value |= (static_cast<uint32_t>(math::clamp(a, -1.0f, 1.0f) * pack_factor) & 0xFFFF);
    value |= (static_cast<uint32_t>(math::clamp(b, -1.0f, 1.0f) * pack_factor) & 0xFFFF) << 16;
    return value;
}
} // namespace math
