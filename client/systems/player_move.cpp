/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>
#include <client/input.hpp>
#include <shared/components/creature.hpp>
#include <shared/components/head.hpp>
#include <common/math/math.hpp>
#include <client/screen.hpp>
#include <client/systems/player_move.hpp>
#include <shared/protocol/packets/shared/creature_position.hpp>
#include <shared/session.hpp>
#include <shared/util/enet.hpp>

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
    
    CreatureComponent &creature = globals::registry.get<CreatureComponent>(globals::session.player_entity);
    const HeadComponent &head = globals::registry.get<HeadComponent>(globals::session.player_entity);
    creature.position += floatquat(math::wrapAngle180N(float3(head.angles.x, head.angles.y, 0.0f))) * direction * 16.0f * globals::frametime;
}

void player_move::send()
{
    const CreatureComponent &creature = globals::registry.get<CreatureComponent>(globals::session.player_entity);
    protocol::packets::CreaturePosition positionp = {};
    positionp.network_id = globals::session.player_network_id;
    math::vecToArray(creature.position, positionp.position);
    util::sendPacket(globals::session.peer, positionp, 0, 0);
}
