/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <common/math/types.hpp>

class Atlas;
class ClientChunkManager;
class GBuffer;
class VoxelDef;
struct GLFWwindow;

namespace cl_globals
{
extern GLFWwindow *window;
extern float curtime;
extern float frametime;
extern float avg_frametime;
extern size_t frame_count;
extern size_t vertices_drawn;
extern entt::registry registry;
extern ClientChunkManager chunks;
extern VoxelDef voxels;
extern Atlas solid_textures;
extern GBuffer solid_gbuffer;
} // namespace cl_globals
