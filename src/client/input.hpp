/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <GLFW/glfw3.h>
#include <math/types.hpp>

namespace input
{
void init(GLFWwindow *window);
void update();

bool isKeyPressed(unsigned int key);
bool isKeyJustPressed(unsigned int key);
bool isKeyJustReleased(unsigned int key);

bool isMouseButtonPressed(unsigned int button);
bool isMouseButtonJustPressed(unsigned int button);
bool isMouseButtonJustReleased(unsigned int button);

const float2_t &getCursor();
const float2_t &getCursorDelta();

const float2_t &getScrollDelta();
} // namespace input
