/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/frustum.hpp>

namespace proj_view
{
void update();
const float3_t &position();
const float4x4_t &matrix();
const Frustum &frustum();
} // namespace proj_view
