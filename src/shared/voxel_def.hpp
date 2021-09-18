/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/types.hpp>
#include <shared/world.hpp>
#include <string>
#include <vector>

namespace voxel_def
{
void add(voxel_t voxel, const VoxelInfo &info);
const VoxelInfo *get(voxel_t voxel);
size_t count();
const voxel_t *list();
} // namespace voxel_def
