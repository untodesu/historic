/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <common/math/types.hpp>

class ServerChunkManager;
class VoxelDef;

namespace sv_globals
{
extern ENetHost *host;
extern uint64_t num_ticks;

extern entt::registry registry;
extern ServerChunkManager chunks;
extern VoxelDef voxels;
} // namespace sv_globals

namespace globals = sv_globals;