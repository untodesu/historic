/*
 * input.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <GLFW/glfw3.h>
#include <common/math/types.hpp>

namespace input
{
void init();
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

const float2 &getCursor();
const float2 &getCursorDelta();

const float2 &getScrollDelta();
} // namespace input
