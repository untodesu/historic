/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/config.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <common/voxels.hpp>

// Configuration
ClientConfig client_globals::config = ClientConfig();

// Rendering
GLFWwindow *client_globals::window = nullptr;
Atlas client_globals::terrain_atlas;
GBuffer client_globals::main_gbuffer;

// Stats
float client_globals::curtime = 0.0f;
float client_globals::frametime = 0.0f;
float client_globals::frametime_avg = 0.0f;
size_t client_globals::frame_count = 0;
size_t client_globals::vertices_drawn = 0;

// World
entt::registry client_globals::registry;
ClientChunkManager client_globals::chunks;
VoxelDef client_globals::voxels;
