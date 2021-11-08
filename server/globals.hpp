/*
 * globals.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <common/math/types.hpp>

class CVarList;
class ServerChunkManager;
class VoxelDef;
struct Session;
class ScriptEngine;

namespace sv_globals
{
extern bool running;

// Networking
extern ENetHost *host;

// World
extern entt::registry registry;
extern ServerChunkManager chunks;
extern VoxelDef voxels;

// Scripting & configuration
extern CVarList cvars;
extern ScriptEngine script;

// Stats
extern float curtime;
extern float ticktime;
extern uint64_t num_ticks;
} // namespace sv_globals

namespace globals = sv_globals;
