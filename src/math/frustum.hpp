/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <array>
#include <math/types.hpp>

class Frustum final {
public:
    struct Plane final {
        float d;
        float3 n;
        float point(const float3 &v) const;
    };

public:
    void update(const float4x4 &projview);
    bool point(const float3 &v) const;

private:
    std::array<Plane, 6> planes;
};
