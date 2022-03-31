/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/world.hpp>

struct ChunkComponent final {
    chunk_pos_t cpos { 0, 0, 0 };
    ChunkComponent() = default;
    ChunkComponent(const chunk_pos_t &cpos) : cpos(cpos) {}
};
