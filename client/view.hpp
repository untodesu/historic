/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/frustum.hpp>

namespace view
{
void update();
const vector2f_t &angles();
const vector3f_t &position();
const matrix4f_t &viewProjectionMatrix();
const math::Frustum &frustum();
} // namespace view
