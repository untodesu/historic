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

float math::FPlane::point(const vector3f_t &v) const
{
    return glm::dot(v, n) + d;
}

math::Frustum::Frustum()
{
    set(MATRIX4F_IDENT);
}

math::Frustum::Frustum(const matrix4f_t &vpmatrix)
{
    set(vpmatrix);
}

void math::Frustum::set(const matrix4f_t &vpmatrix)
{
    planes[PLANE_LF].n.x = vpmatrix[0][3] + vpmatrix[0][0];
    planes[PLANE_LF].n.y = vpmatrix[1][3] + vpmatrix[1][0];
    planes[PLANE_LF].n.z = vpmatrix[2][3] + vpmatrix[2][0];
    planes[PLANE_LF].d = vpmatrix[3][3] + vpmatrix[3][0];

    planes[PLANE_RT].n.x = vpmatrix[0][3] - vpmatrix[0][0];
    planes[PLANE_RT].n.y = vpmatrix[1][3] - vpmatrix[1][0];
    planes[PLANE_RT].n.z = vpmatrix[2][3] - vpmatrix[2][0];
    planes[PLANE_RT].d = vpmatrix[3][3] - vpmatrix[3][0];

    planes[PLANE_FT].n.x = vpmatrix[0][3] + vpmatrix[0][2];
    planes[PLANE_FT].n.y = vpmatrix[1][3] + vpmatrix[1][2];
    planes[PLANE_FT].n.z = vpmatrix[2][3] + vpmatrix[2][2];
    planes[PLANE_FT].d = vpmatrix[3][3] + vpmatrix[3][2];

    planes[PLANE_BK].n.x = vpmatrix[0][3] - vpmatrix[0][2];
    planes[PLANE_BK].n.y = vpmatrix[1][3] - vpmatrix[1][2];
    planes[PLANE_BK].n.z = vpmatrix[2][3] - vpmatrix[2][2];
    planes[PLANE_BK].d = vpmatrix[3][3] - vpmatrix[3][2];

    planes[PLANE_DN].n.x = vpmatrix[0][3] + vpmatrix[0][1];
    planes[PLANE_DN].n.y = vpmatrix[1][3] + vpmatrix[1][1];
    planes[PLANE_DN].n.z = vpmatrix[2][3] + vpmatrix[2][1];
    planes[PLANE_DN].d = vpmatrix[3][3] + vpmatrix[3][1];

    planes[PLANE_UP].n.x = vpmatrix[0][3] - vpmatrix[0][1];
    planes[PLANE_UP].n.y = vpmatrix[1][3] - vpmatrix[1][1];
    planes[PLANE_UP].n.z = vpmatrix[2][3] - vpmatrix[2][1];
    planes[PLANE_UP].d = vpmatrix[3][3] - vpmatrix[3][1];

    for(math::FPlane &plane : planes) {
        if(float length = glm::length(plane.n)) {
            plane.n /= length;
            plane.d /= length;
        }
    }
}

bool math::Frustum::point(const vector3f_t &v) const
{
    for(const math::FPlane &plane : planes) {
        if(plane.point(v) >= 0.0f)
            continue;
        return false;
    }

    return true;
}
