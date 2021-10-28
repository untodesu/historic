/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/comp/local_player.hpp>
#include <game/client/sys/player_look.hpp>
#include <game/client/globals.hpp>
#include <game/client/input.hpp>
#include <game/client/screen.hpp>
#include <game/shared/comp/head.hpp>
#include <game/shared/comp/player.hpp>
#include <common/math/const.hpp>
#include <common/math/math.hpp>

void player_look::update()
{
    // FIXME: sensitivity is higher when the FPS is higher.
    // UNDONE: FPS-independent mouse look (hard but possible, I guess).
    const float2 delta = input::getCursorDelta() * 8.0f / screen::getSize();
    auto hg = globals::registry.group(entt::get<LocalPlayerComponent, HeadComponent, PlayerComponent>);
    for(auto [entity, head, player] : hg.each()) {
        head.angles.x -= delta.y;
        head.angles.y -= delta.x;
        head.angles.x = math::clamp(head.angles.x, -ANGLE_89D, ANGLE_89D);
        head.angles = math::wrapAngle180N(head.angles);
        return;
    }
}
