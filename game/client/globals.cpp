/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
#include <game/client/atlas.hpp>
#include <game/client/chunks.hpp>
#include <game/client/gbuffer.hpp>
#include <game/client/globals.hpp>
#include <game/shared/voxels.hpp>

// Networking
ENetHost *cl_globals::host = nullptr;
ENetPeer *cl_globals::peer = nullptr;
uint32_t cl_globals::session_id = 0;
ClientState cl_globals::state = ClientState::DISCONNECTED;

// Rendering
GLFWwindow *cl_globals::window = nullptr;
Atlas cl_globals::solid_textures;
GBuffer cl_globals::solid_gbuffer;

// World
entt::entity cl_globals::local_player;
entt::registry cl_globals::registry;
ClientChunkManager cl_globals::chunks;
VoxelDef cl_globals::voxels;

// Stats
float cl_globals::curtime = 0.0f;
float cl_globals::frametime = 0.0f;
float cl_globals::avg_frametime = 0.0f;
size_t cl_globals::frame_count = 0;
size_t cl_globals::vertices_drawn = 0;
