/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/local_player.hpp>
#include <client/sys/player_move.hpp>
#include <client/client_world.hpp>
#include <client/input.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <math/util.hpp>

void player_move::update(float frametime)
{
    float3_t direction = FLOAT3_ZERO;

    if(input::isKeyPressed(GLFW_KEY_W))
        direction += FLOAT3_FORWARD;
    if(input::isKeyPressed(GLFW_KEY_S))
        direction -= FLOAT3_FORWARD;
    if(input::isKeyPressed(GLFW_KEY_D))
        direction += FLOAT3_RIGHT;
    if(input::isKeyPressed(GLFW_KEY_A))
        direction -= FLOAT3_RIGHT;

    if(input::isKeyPressed(GLFW_KEY_SPACE))
        direction += FLOAT3_UP;
    if(input::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        direction -= FLOAT3_UP;

    auto pg = client_world::registry().group(entt::get<LocalPlayerComponent, CreatureComponent, HeadComponent, PlayerComponent>);
    for(auto [entity, creature, head] : pg.each()) {
        creature.position += floatquat_t(math_util::fixAngle180(head.angles)) * direction * 8.0f * frametime;
        return;
    }
}
