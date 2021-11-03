/*
 * camera.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once

struct ActiveCameraComponent final {};
struct CameraComponent final {
    float fov;
    float z_far;
    float z_near;
};
