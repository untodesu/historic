/*
 * console.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>

namespace console
{
void preInit();
void init();
void postInit();
void update();
void drawImgui();
} // namespace console
