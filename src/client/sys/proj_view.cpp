/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/sys/proj_view.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>

static float3_t pv_position = FLOAT3_ZERO;
static float4x4_t pv_matrix = FLOAT4X4_IDENTITY;
static Frustum pv_frustum;

void proj_view::update()
{
    pv_position = FLOAT3_ZERO;
    pv_matrix = FLOAT4X4_IDENTITY;

    const auto cg = cl_globals::registry.group<ActiveCameraComponent>(entt::get<CameraComponent>);
    for(const auto [entity, camera] : cg.each()) {
        pv_matrix *= glm::perspective(camera.fov, screen::getAspectRatio(), camera.z_near, camera.z_far);
        break;
    }

    const auto hg = cl_globals::registry.group(entt::get<LocalPlayerComponent, HeadComponent, PlayerComponent>);
    for(const auto [entity, head] : hg.each()) {
        pv_position = head.offset;
        if(CreatureComponent *creature = cl_globals::registry.try_get<CreatureComponent>(entity))
            pv_position += creature->position;
        pv_matrix *= glm::lookAt(pv_position, pv_position + floatquat_t(head.angles) * FLOAT3_FORWARD, FLOAT3_UP);
        break;
    }

    pv_frustum.update(pv_matrix);
}

const float3_t &proj_view::position()
{
    return pv_position;
}

const float4x4_t &proj_view::matrix()
{
    return pv_matrix;
}

const Frustum &proj_view::frustum()
{
    return pv_frustum;
}
