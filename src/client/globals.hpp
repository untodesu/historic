/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/atlas.hpp>
#include <entt/entt.hpp>
#include <math/types.hpp>
#include <shared/voxels.hpp>
#include <uvre/fwd.hpp>

namespace globals
{
extern float curtime;
extern float frametime;
extern uint64_t frame_count;
extern uvre::IRenderDevice *render_device;
extern entt::registry registry;
extern VoxelDef voxels;
extern Atlas solid_textures;
} // namespace globals
