/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/types.hpp>
#include <entt/entt.hpp>

class Atlas;
class ClientChunkManager;
class ClientConfig;
class GBuffer;
class VoxelDef;
struct GLFWwindow;

namespace client_globals
{
// Configuration
extern ClientConfig config;

// Rendering
extern GLFWwindow *window;
extern Atlas terrain_atlas;
extern GBuffer main_gbuffer;

// Stats
extern float curtime;
extern float frametime;
extern float frametime_avg;
extern size_t frame_count;
extern size_t vertices_drawn;

// World
extern entt::registry registry;
extern ClientChunkManager chunks;
extern VoxelDef voxels;
} // namespace client_globals
namespace globals = client_globals;
