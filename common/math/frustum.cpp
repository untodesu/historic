/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/frustum.hpp>

using plane_t = unsigned short;
constexpr static const plane_t PLANE_LF = 0;
constexpr static const plane_t PLANE_RT = 1;
constexpr static const plane_t PLANE_FT = 2;
constexpr static const plane_t PLANE_BK = 3;
constexpr static const plane_t PLANE_UP = 4;
constexpr static const plane_t PLANE_DN = 5;

float math::fplane::point(const vector3f_t &v) const
{
    return glm::dot(v, n) + d;
}

void math::Frustum::set(const matrix4f_t &projview)
{
    planes[PLANE_LF].n.x = projview[0][3] + projview[0][0];
    planes[PLANE_LF].n.y = projview[1][3] + projview[1][0];
    planes[PLANE_LF].n.z = projview[2][3] + projview[2][0];
    planes[PLANE_LF].d = projview[3][3] + projview[3][0];

    planes[PLANE_RT].n.x = projview[0][3] - projview[0][0];
    planes[PLANE_RT].n.y = projview[1][3] - projview[1][0];
    planes[PLANE_RT].n.z = projview[2][3] - projview[2][0];
    planes[PLANE_RT].d = projview[3][3] - projview[3][0];

    planes[PLANE_FT].n.x = projview[0][3] + projview[0][2];
    planes[PLANE_FT].n.y = projview[1][3] + projview[1][2];
    planes[PLANE_FT].n.z = projview[2][3] + projview[2][2];
    planes[PLANE_FT].d = projview[3][3] + projview[3][2];

    planes[PLANE_BK].n.x = projview[0][3] - projview[0][2];
    planes[PLANE_BK].n.y = projview[1][3] - projview[1][2];
    planes[PLANE_BK].n.z = projview[2][3] - projview[2][2];
    planes[PLANE_BK].d = projview[3][3] - projview[3][2];

    planes[PLANE_DN].n.x = projview[0][3] + projview[0][1];
    planes[PLANE_DN].n.y = projview[1][3] + projview[1][1];
    planes[PLANE_DN].n.z = projview[2][3] + projview[2][1];
    planes[PLANE_DN].d = projview[3][3] + projview[3][1];

    planes[PLANE_UP].n.x = projview[0][3] - projview[0][1];
    planes[PLANE_UP].n.y = projview[1][3] - projview[1][1];
    planes[PLANE_UP].n.z = projview[2][3] - projview[2][1];
    planes[PLANE_UP].d = projview[3][3] - projview[3][1];

    for(math::fplane &plane : planes) {
        if(float length = glm::length(plane.n)) {
            plane.n /= length;
            plane.d /= length;
        }
    }
}

bool math::Frustum::point(const vector3f_t &v) const
{
    for(const math::fplane &plane : planes) {
        if(plane.point(v) >= 0.0f)
            continue;
        return false;
    }

    return true;
}
