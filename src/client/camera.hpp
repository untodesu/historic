/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math_defs.hpp>

struct ActiveCameraComponent {};
struct ControlledCameraComponent {};
struct CameraComponent {
    float z_near, z_far;
    float aspect;
    float fov;
    float3_t offset { FLOAT3_ZERO };
    float3_t angles { FLOAT3_ZERO };
};
