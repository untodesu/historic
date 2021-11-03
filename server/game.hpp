/*
 * game.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once

namespace sv_game
{
void init();
void postInit();
void shutdown();
void update();
} // namespace sv_game

namespace game = sv_game;
