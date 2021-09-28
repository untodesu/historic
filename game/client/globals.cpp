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

ENetHost *globals::host = nullptr;
ENetPeer *globals::peer = nullptr;
GLFWwindow *globals::window = nullptr;
float globals::curtime = 0.0f;
float globals::frametime = 0.0f;
float globals::avg_frametime = 0.0f;
size_t globals::frame_count = 0;
size_t globals::vertices_drawn = 0;
entt::registry globals::registry;
ClientChunkManager globals::chunks;
VoxelDef globals::voxels;
Atlas globals::solid_textures;
GBuffer globals::solid_gbuffer;
