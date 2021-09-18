/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/local_player.hpp>
#include <client/sys/player_look.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <math/util.hpp>

void player_look::update()
{
    // FIXME: sensitivity is higher when the FPS is higher.
    // UNDONE: FPS-independent mouse look (hard but possible, I guess).
    const float2_t delta = input::getCursorDelta() * 0.025f;
    auto hg = globals::registry.group(entt::get<LocalPlayerComponent, HeadComponent, PlayerComponent>);
    for(auto [entity, head] : hg.each()) {
        head.angles.x -= delta.y;
        head.angles.y -= delta.x;
        head.angles.x = math::clamp(head.angles.x, glm::radians(-89.0f), glm::radians(89.0f));
        return;
    }
}
