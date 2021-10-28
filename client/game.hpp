/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <string>

namespace cl_game
{
void init();
void postInit();
void shutdown();
void modeChange(int width, int height);
bool connect(const std::string &host, uint16_t port);
void disconnect(const std::string &reason);
void update();
void draw();
void drawImgui();
void postDraw();
} // namespace cl_game

namespace game = cl_game;
