/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <shared/voxels.hpp>

// Networking
ENetHost *cl_globals::host = nullptr;
ENetPeer *cl_globals::peer = nullptr;
uint32_t cl_globals::session_id = 0;
ClientState cl_globals::state = ClientState::DISCONNECTED;
std::unordered_map<uint16_t, cl_packet_handler_t> cl_globals::packet_handlers;

// Rendering
GLFWwindow *cl_globals::window = nullptr;
Atlas cl_globals::solid_textures;
GBuffer cl_globals::solid_gbuffer;

// World
uint32_t cl_globals::local_player_network_id = 0;
entt::entity cl_globals::local_player = entt::null;
entt::registry cl_globals::registry;
ClientChunkManager cl_globals::chunks;
VoxelDef cl_globals::voxels;

// Stats
float cl_globals::curtime = 0.0f;
float cl_globals::frametime = 0.0f;
float cl_globals::avg_frametime = 0.0f;
size_t cl_globals::frame_count = 0;
size_t cl_globals::vertices_drawn = 0;
