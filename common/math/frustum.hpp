/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <array>
#include <common/types.hpp>

namespace math
{
struct fplane final {
    float d;
    vector3f_t n;
    float point(const vector3f_t &v) const;
};

class Frustum final {
public:
    Frustum();
    Frustum(const matrix4f_t &matrix);
    void set(const matrix4f_t &matrix);
    bool point(const vector3f_t &v) const;

private:
    std::array<math::fplane, 6> planes;
};
} // namespace math
