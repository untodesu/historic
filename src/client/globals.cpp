/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/globals.hpp>
#include <shared/voxels.hpp>

float cl_globals::curtime = 0.0f;
float cl_globals::frametime = 0.0f;
uint64_t cl_globals::frame_count = 0;
entt::registry cl_globals::registry = entt::registry();
ClientChunkManager cl_globals::chunks = ClientChunkManager();
VoxelDef cl_globals::voxels = VoxelDef();
Atlas cl_globals::solid_textures = Atlas();
