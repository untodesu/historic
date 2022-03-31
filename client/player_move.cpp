/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/player_move.hpp>
#include <common/comp/creature_component.hpp>
#include <common/comp/player_component.hpp>
#include <common/math/angle_wrap.hpp>

void player_move::update()
{
    vector3f_t move_dir = VECTOR3F_ZERO;
    if(input::isKeyPressed(GLFW_KEY_W))
        move_dir += VECTOR3F_FORWARD;
    if(input::isKeyPressed(GLFW_KEY_S))
        move_dir -= VECTOR3F_FORWARD;
    if(input::isKeyPressed(GLFW_KEY_D))
        move_dir += VECTOR3F_RIGHT;
    if(input::isKeyPressed(GLFW_KEY_A))
        move_dir -= VECTOR3F_RIGHT;
    if(input::isKeyPressed(GLFW_KEY_SPACE))
        move_dir += VECTOR3F_UP;
    if(input::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        move_dir -= VECTOR3F_UP;

    const auto group = globals::registry.group(entt::get<CreatureComponent, PlayerComponent>);
    for(const auto [entity, creature, player] : group.each()) {
        creature.vpos += quaternf_t(math::wrapAngle180N(vector3f_t(player.head_angles.x, player.head_angles.y, 0.0f))) * move_dir * 16.0f * globals::frametime;
        break;
    }
}
