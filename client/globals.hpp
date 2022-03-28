/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <core/types.hpp>
#include <entt/entt.hpp>

class ClientConfig;
struct GLFWwindow;

namespace client_globals
{
// Configuration
extern ClientConfig config;

// Rendering
extern GLFWwindow *window;

// Stats
extern float curtime;
extern float frametime;
extern float frametime_avg;
extern size_t frame_count;
} // namespace client_globals
namespace globals = client_globals;
