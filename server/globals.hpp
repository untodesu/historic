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
#include <vector>
#include <unordered_map>

class ServerChunkManager;
class VoxelDef;
struct Session;

using sv_packet_handler_t = void(*)(const std::vector<uint8_t> &, Session *);

namespace sv_globals
{
// Networking
extern ENetHost *host;
extern std::unordered_map<uint16_t, sv_packet_handler_t> packet_handlers;

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
