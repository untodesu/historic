/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/comp/local_player.hpp>
#include <game/client/sys/player_move.hpp>
#include <game/client/globals.hpp>
#include <game/client/input.hpp>
#include <game/shared/comp/creature.hpp>
#include <game/shared/comp/head.hpp>
#include <game/shared/comp/player.hpp>
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <game/client/chunks.hpp>
#include <spdlog/spdlog.h>

void player_move::update()
{
    float3 direction = FLOAT3_ZERO;

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

    auto pg = globals::registry.group(entt::get<LocalPlayerComponent, CreatureComponent, HeadComponent, PlayerComponent>);
    for(auto [entity, creature, head, player] : pg.each()) {
        creature.position += floatquat(math::wrapAngle180N(float3(head.angles.x, head.angles.y, 0.0f))) * direction * 16.0f * globals::frametime;
        if(input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))
            globals::chunks.set(toVoxelPos(glm::floor(creature.position + head.offset)), NULL_VOXEL, VOXEL_SET_UPDATE_NEIGHBOURS);
        return;
    }
}
