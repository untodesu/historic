/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <math/const.hpp>
#include <shared/voxels.hpp>

float cl_globals::curtime = 0.0f;
float cl_globals::frametime = 0.0f;
size_t cl_globals::frame_count = 0;
size_t cl_globals::vertices_drawn = 0;
entt::registry cl_globals::registry;
ClientChunkManager cl_globals::chunks;
VoxelDef cl_globals::voxels;
Atlas cl_globals::solid_textures;
GBuffer cl_globals::solid_gbuffer;
float3 cl_globals::shadow_angles = FLOAT3_ZERO;
