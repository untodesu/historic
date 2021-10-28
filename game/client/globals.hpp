/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <vector>
#include <unordered_map>

enum class ClientState {
    DISCONNECTED,
    LOGGING_IN,
    RECEIVING_GAME_DATA,
    PLAYING,
    PAUSED
};

struct GLFWwindow;
class ClientChunkManager;
class VoxelDef;
class Atlas;
class GBuffer;

using cl_packet_handler_t = void(*)(const std::vector<uint8_t> &);

namespace cl_globals
{
// Networking
extern ENetHost *host;
extern ENetPeer *peer;
extern uint32_t session_id;
extern ClientState state;
extern std::unordered_map<uint16_t, cl_packet_handler_t> packet_handlers;

// Rendering
extern GLFWwindow *window;
extern Atlas solid_textures;
extern GBuffer solid_gbuffer;

// World
extern entt::entity local_player;
extern entt::registry registry;
extern ClientChunkManager chunks;
extern VoxelDef voxels;

// Stats
extern float curtime;
extern float frametime;
extern float avg_frametime;
extern size_t frame_count;
extern size_t vertices_drawn;
} // namespace cl_globals

namespace globals = cl_globals;
