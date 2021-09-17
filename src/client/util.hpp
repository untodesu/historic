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
voxel_t getVoxel(const voxelpos_t &vp);
voxel_t getVoxel(const chunkpos_t &cp, const localpos_t &lp);
} // namespace client_util
