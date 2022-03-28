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
static inline const angle_t wrapAngle180N(const angle_t angle)
{
    const angle_t wrap = glm::mod(angle + ANGLE_180D, ANGLE_360D);
    return ((wrap < 0.0f) ? (wrap + ANGLE_360D) : wrap) - ANGLE_180D;
}

static inline const angle_t wrapAngle180P(const angle_t angle)
{
    return glm::mod(glm::mod(angle, ANGLE_180D) + ANGLE_180D, ANGLE_180D);
}

static inline const angle_t wrapAngle360P(const angle_t angle)
{
    return glm::mod(glm::mod(angle, ANGLE_360D) + ANGLE_360D, ANGLE_360D);
}

template<glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, angle_t, Q> wrapAngle180N(const glm::vec<L, angle_t, Q> &angles)
{
    glm::vec<L, angle_t, Q> wrap;
    for(glm::length_t i = 0; i < L; i++)
        wrap[i] = math::wrapAngle180N(angles[i]);
    return wrap;
}

template<glm::length_t L, glm::qualifier Q>
static inline const glm::vec<L, angle_t, Q> wrapAngle360P(const glm::vec<L, angle_t, Q> &angles)
{
    glm::vec<L, angle_t, Q> wrap;
    for(glm::length_t i = 0; i < L; i++)
        wrap[i] = math::wrapAngle360P(angles[i]);
    return wrap;
}
} // namespace math
