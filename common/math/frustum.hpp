/*
 * frustum.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <array>
#include <common/math/types.hpp>

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
