/*
 * game.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>
#include <string>

namespace cl_game
{
void preInit();
void init();
void postInit();
void shutdown();
void modeChange(int width, int height);
void update();
void draw();
void drawImgui();
void postDraw();
} // namespace cl_game

namespace game = cl_game;
