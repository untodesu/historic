/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/types.hpp>

struct PlayerComponent final {
    vector2f_t head_angles {VECTOR2F_ZERO};
    vector3f_t head_offset {VECTOR3F_ZERO};
};
