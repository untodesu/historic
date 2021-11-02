/*
 * player_look.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/globals.hpp>
#include <client/input.hpp>
#include <shared/components/head.hpp>
#include <common/math/math.hpp>
#include <client/screen.hpp>
#include <client/systems/player_look.hpp>
#include <shared/protocol/packets/shared/head_angles.hpp>
#include <shared/session.hpp>
#include <shared/util/enet.hpp>

void player_look::update()
{
    const float2 delta = input::getCursorDelta() * 8.0f / screen::getSize();
    HeadComponent &head = globals::registry.get<HeadComponent>(globals::session.player_entity);
    head.angles.x -= delta.y;
    head.angles.y -= delta.x;
    head.angles.x = math::clamp(head.angles.x, -ANGLE_89D, ANGLE_89D);
    head.angles = math::wrapAngle180N(head.angles);
}

void player_look::send()
{
    const float2 delta = input::getCursorDelta() * 8.0f / screen::getSize();
    const HeadComponent &head = globals::registry.get<HeadComponent>(globals::session.player_entity);
    protocol::packets::HeadAngles headp = {};
    headp.network_id = globals::session.player_network_id;
    math::vecToArray(head.angles, headp.angles);
    util::sendPacket(globals::session.peer, headp, 0, 0);
}
