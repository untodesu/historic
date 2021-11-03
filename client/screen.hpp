/*
 * screen.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>

struct GLFWwindow;

namespace screen
{
void init();
float getAspectRatio();
const float2 &getSize();
void getSize(int &width, int &height);
} // namespace screen

