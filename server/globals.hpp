/*
 * globals.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <common/math/types.hpp>

class ServerChunkManager;
class VoxelDef;
struct Session;

namespace sv_globals
{
// Networking
extern ENetHost *host;

// World
extern entt::registry registry;
extern ServerChunkManager chunks;
extern VoxelDef voxels;

// Stats
extern float curtime;
extern float ticktime;
extern uint64_t num_ticks;
} // namespace sv_globals

namespace globals = sv_globals;
