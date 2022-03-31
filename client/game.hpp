/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace client_game
{
void initialize();
void postInitialize();
void shutdown();
void mode(int width, int height);
void update();
void render();
void postRender();
} // namespace client_game
namespace game = client_game;
