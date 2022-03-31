/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/types.hpp>
#include <GLFW/glfw3.h>

namespace input
{
void initialize();
void update();

bool isKeyPressed(unsigned int key);
bool isKeyJustPressed(unsigned int key);
bool isKeyJustReleased(unsigned int key);

bool isMouseButtonPressed(unsigned int button);
bool isMouseButtonJustPressed(unsigned int button);
bool isMouseButtonJustReleased(unsigned int button);

void toggleCursor();
void enableCursor(bool enable);
bool cursorEnabled();

const vector2f_t &getCursor();
const vector2f_t &getCursorDelta();

const vector2f_t &getScrollDelta();
} // namespace input
