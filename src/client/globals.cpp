/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>

float globals::curtime = 0.0f;
float globals::frametime = 0.0f;
uint64_t globals::frame_count = 0;
entt::registry globals::registry = entt::registry();
uvre::IRenderDevice *globals::render_device = nullptr;
VoxelDef globals::voxels = VoxelDef();
Atlas globals::solid_textures = Atlas();
