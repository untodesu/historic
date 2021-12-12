/*
 * globals.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>
#include <enet/enet.h>
#include <entt/entt.hpp>

struct ClientSession;
struct GLFWwindow;
class ClientConfig;
class ClientChunkManager;
class VoxelDef;
class Atlas;
class GBuffer;
class ScriptEngine;

namespace cl_globals
{
// Networking
extern ENetHost *host;
extern ClientSession session;

// Rendering
extern GLFWwindow *window;
extern Atlas solid_textures;
extern GBuffer solid_gbuffer;
extern bool ui_grabs_input;

// World
extern uint32_t local_player_network_id;
extern entt::entity local_player;
extern entt::registry registry;
extern ClientChunkManager chunks;
extern VoxelDef voxels;

// Configuration
extern ClientConfig config;

// Stats
extern float curtime;
extern float frametime;
extern float avg_frametime;
extern size_t frame_count;
extern size_t vertices_drawn;
} // namespace cl_globals

namespace globals = cl_globals;
