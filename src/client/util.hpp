/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <shared/world.hpp>

namespace client_util
{
bool getChunk(const chunkpos_t &cp, voxel_array_t &out);
voxel_array_t *getChunk(const chunkpos_t &cp);
} // namespace client_util
