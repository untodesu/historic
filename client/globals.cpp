/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/config.hpp>
#include <client/globals.hpp>

// Configuration
ClientConfig client_globals::config = ClientConfig();

// Rendering
GLFWwindow *client_globals::window = nullptr;

// Stats
float client_globals::curtime = 0.0f;
float client_globals::frametime = 0.0f;
float client_globals::frametime_avg = 0.0f;
size_t client_globals::frame_count = 0;
