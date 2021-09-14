/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/camera.hpp>
#include <client/camera_controller.hpp>
#include <client/input.hpp>
#include <math_defs.hpp>
#include <shared/transform.hpp>
#include <spdlog/spdlog.h>

// TODO: quake-ish CVar system to handle this.
constexpr static const float SENSITIVITY = 1.0f;

static float4x4_t projview = FLOAT4X4_IDENTITY;

void camera_controller::update(entt::registry &registry, float frametime)
{
    // Update controlled cameras
    const float2_t delta = input::getCursorDelta() * SENSITIVITY * frametime * -1.0f;
    auto controlled_group = registry.group<ControlledCameraComponent>(entt::get<CameraComponent>);
    for(auto [entity, camera] : controlled_group.each()) {
        camera.angles.y += delta.x;
        camera.angles.x += delta.y;
        camera.angles.x = glm::clamp(camera.angles.x, ANG_89NEG, ANG_89POS);
    }

    const auto active_group = registry.group<ActiveCameraComponent>(entt::get<CameraComponent>);
    for(const auto [entity, camera] : active_group.each()) {
        float3_t position = camera.offset;
        floatquat_t orientation = floatquat_t(camera.angles);

        // If the entity happens to also have a transform component,
        // the position and orientation values are affected by it.
        if(const TransformComponent *transform = registry.try_get<TransformComponent>(entity)) {
            position += transform->position;
            orientation *= transform->orientation;
        }

        projview = glm::perspective(camera.fov, camera.aspect, camera.z_near, camera.z_far) * glm::lookAt(position, position + orientation * FLOAT3_FORWARD, FLOAT3_UP);

        // There shall be only one active camera
        // during this frame, so we iterate once.
        return;
    }

    spdlog::warn("No active cameras during frame!");
    projview = FLOAT4X4_IDENTITY;
}

const float4x4_t &camera_controller::projviewMatrix()
{
    return projview;
}
