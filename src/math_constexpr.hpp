/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace math
{
template<typename T>
static inline constexpr const T log2floor(const T x)
{
    if(x <= 1)
        return 0;
    return math::log2floor<T>(x >> 1) + 1;
}

template<typename T>
static inline constexpr const T log2ceil(const T x)
{
    if(x <= 1)
        return 0;
    return math::log2ceil((x + 1) >> 1) + 1;
}

template<typename T>
static inline constexpr const T log2(const T x)
{
    return math::log2ceil<T>(x);
}
} // namespace math
