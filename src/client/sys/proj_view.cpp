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
#include <client/shadow_manager.hpp>
#include <client/screen.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <shared/world.hpp>

static float3 pv_position = FLOAT3_ZERO;
static float4x4 pv_matrix = FLOAT4X4_IDENTITY;
static float4x4 pv_matrix_shadow = FLOAT4X4_IDENTITY;
static Frustum pv_frustum, pv_frustum_shadow;

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
        pv_matrix *= glm::lookAt(pv_position, pv_position + floatquat(head.angles) * FLOAT3_FORWARD, FLOAT3_UP);
        break;
    }

    pv_frustum.update(pv_matrix);

    pv_matrix_shadow = shadow_manager::matrix(pv_position);
    pv_frustum_shadow.update(pv_matrix_shadow);
}

const float3 &proj_view::position()
{
    return pv_position;
}

const float4x4 &proj_view::matrix()
{
    return pv_matrix;
}

const float4x4 &proj_view::matrixShadow()
{
    return pv_matrix_shadow;
}

const Frustum &proj_view::frustum()
{
    return pv_frustum;
}

const Frustum &proj_view::frustumShadow()
{
    return pv_frustum_shadow;
}
