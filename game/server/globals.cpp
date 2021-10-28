/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/chunks.hpp>
#include <game/server/globals.hpp>
#include <game/shared/voxels.hpp>

// Networking
ENetHost *sv_globals::host = nullptr;
std::unordered_map<uint16_t, sv_packet_handler_t> sv_globals::packet_handlers;

// World
entt::registry sv_globals::registry;
ServerChunkManager sv_globals::chunks;
VoxelDef sv_globals::voxels;

// Stats
float sv_globals::curtime = 0.0f;
float sv_globals::ticktime = 0.0f;
uint64_t sv_globals::num_ticks = 0;
