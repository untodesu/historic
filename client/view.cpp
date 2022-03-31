/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/view.hpp>
#include <common/comp/creature_component.hpp>
#include <common/comp/player_component.hpp>

static vector2f_t view_angles = VECTOR2F_ZERO;
static vector3f_t view_position = VECTOR3F_ZERO;
static matrix4f_t view_vpmatrix = MATRIX4F_IDENT;
static math::Frustum view_frustum;

void view::update()
{
    view_angles = VECTOR2F_ZERO;
    view_position = VECTOR3F_ZERO;
    view_vpmatrix = glm::perspective(glm::radians(90.0f), screen::aspect(), 0.01f, 1024.0f);

    const auto group = globals::registry.group(entt::get<CreatureComponent, PlayerComponent>);
    for(const auto [entity, creature, player] : group.each()) {
        view_angles = player.head_angles;
        view_position = player.head_offset + creature.vpos;
        view_vpmatrix *= glm::lookAt(view_position, view_position + quaternf_t(vector3f_t(view_angles.x, view_angles.y, 0.0f)) * VECTOR3F_FORWARD, VECTOR3F_UP);
        break;
    }

    view_frustum.set(view_vpmatrix);
}

const vector2f_t &view::angles()
{
    return view_angles;
}

const vector3f_t &view::position()
{
    return view_position;
}

const matrix4f_t &view::viewProjectionMatrix()
{
    return view_vpmatrix;
}

const math::Frustum &view::frustum()
{
    return view_frustum;
}
