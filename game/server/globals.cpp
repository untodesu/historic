/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/chunks.hpp>
#include <game/server/globals.hpp>
#include <game/shared/voxels.hpp>

ENetHost *sv_globals::host = nullptr;
uint64_t sv_globals::num_ticks = 0;

entt::registry sv_globals::registry;
ServerChunkManager sv_globals::chunks;
VoxelDef sv_globals::voxels;
