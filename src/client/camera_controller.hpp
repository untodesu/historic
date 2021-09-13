/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <math_types.hpp>

namespace camera_controller
{
void update(entt::registry &registry, float frametime);
const float4x4_t &projviewMatrix();
} // namespace camera_controller
