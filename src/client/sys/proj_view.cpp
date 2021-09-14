/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/sys/proj_view.hpp>
#include <client/client_world.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>

static float4x4_t matrix = FLOAT4X4_IDENTITY;

void proj_view::update()
{
    entt::registry &registry = client_world::registry();

    matrix = FLOAT4X4_IDENTITY;

    const auto cg = registry.group<ActiveCameraComponent>(entt::get<CameraComponent>);
    for(const auto [entity, camera] : cg.each()) {
        matrix *= glm::perspective(camera.fov, screen::getAspectRatio(), camera.z_near, camera.z_far);
        break;
    }

    const auto hg = registry.group(entt::get<LocalPlayerComponent, HeadComponent, PlayerComponent>);
    for(const auto [entity, head] : hg.each()) {
        float3_t position = head.offset;
        if(CreatureComponent *creature = registry.try_get<CreatureComponent>(entity))
            position += creature->position;
        matrix *= glm::lookAt(position, position + floatquat_t(head.angles) * FLOAT3_FORWARD, FLOAT3_UP);
        break;
    }
}

const float4x4_t &proj_view::get()
{
    return matrix;
}
