/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/player_look.hpp>
#include <client/screen.hpp>
#include <common/comp/player_component.hpp>
#include <common/math/angle_wrap.hpp>
#include <common/math/constexpr.hpp>

void player_look::update()
{
    const vector2f_t delta = input::getCursorDelta() * 8.0f / screen::size2f();
    const auto view = globals::registry.view<PlayerComponent>();
    for(const auto [entity, player] : view.each()) {
        player.head_angles.x -= delta.y;
        player.head_angles.y -= delta.x;
        player.head_angles.x = math::clamp(player.head_angles.x, -ANGLE_89D, ANGLE_89D);
        player.head_angles = math::wrapAngle180N(player.head_angles);
        break;
    }
}
