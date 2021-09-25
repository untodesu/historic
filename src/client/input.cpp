/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/input.hpp>
#include <limits>
#include <math/const.hpp>
#include <spdlog/spdlog.h>

static constexpr const size_t NUM_KEYS = GLFW_KEY_LAST + 1;
static constexpr const size_t NUM_MOUSE_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;
static constexpr const unsigned int UNKNOWN_VALUE = std::numeric_limits<unsigned int>::max();

static bool keys[NUM_KEYS] = { 0 };
static unsigned int last_key_press = UNKNOWN_VALUE;
static unsigned int last_key_release = UNKNOWN_VALUE;

static bool mouse_buttons[NUM_MOUSE_BUTTONS] = { 0 };
static unsigned int last_mouse_button_press = UNKNOWN_VALUE;
static unsigned int last_mouse_button_release = UNKNOWN_VALUE;

static float2 cursor = FLOAT2_ZERO;
static float2 cursor_delta = FLOAT2_ZERO;

static float2 scroll_delta = FLOAT2_ZERO;

static void onKey(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // It was crashing on multiple Alt+PrintScreen
    // combinations because for some reason key was
    // GLFW_KEY_UNKNOWN. Strange...
    if(key != GLFW_KEY_UNKNOWN) {
        switch(action) {
            case GLFW_PRESS:
                keys[key] = true;
                last_key_press = key;
                break;
            case GLFW_RELEASE:
                keys[key] = false;
                last_key_release = key;
                break;
        }
    }
}

static void onMouseButton(GLFWwindow *window, int button, int action, int mods)
{
    switch(action) {
        case GLFW_PRESS:
            mouse_buttons[button] = true;
            last_mouse_button_press = button;
            break;
        case GLFW_RELEASE:
            mouse_buttons[button] = false;
            last_mouse_button_release = button;
            break;
    }
}

static void onCursorPos(GLFWwindow *window, double x, double y)
{
    const float fx = static_cast<float>(x);
    const float fy = static_cast<float>(y);

    cursor_delta.x = fx - cursor.x;
    cursor_delta.y = fy - cursor.y;

    cursor.x = fx;
    cursor.y = fy;
}

static void onScroll(GLFWwindow *window, double dx, double dy)
{
    scroll_delta.x = static_cast<float>(dx);
    scroll_delta.y = static_cast<float>(dy);
}

void input::init(GLFWwindow *window)
{
    spdlog::debug("Hooking input events");

    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    // Do a pre-update to make sure
    // our last pressed/released fields
    // are set to UNKNOWN_VALUE
    input::update();
}

void input::update()
{
    last_key_press = UNKNOWN_VALUE;
    last_key_release = UNKNOWN_VALUE;
    last_mouse_button_press = UNKNOWN_VALUE;
    last_mouse_button_release = UNKNOWN_VALUE;
    cursor_delta = FLOAT2_ZERO;
    scroll_delta = FLOAT2_ZERO;
}

bool input::isKeyPressed(unsigned int key)
{
    return keys[key % NUM_KEYS];
}

bool input::isKeyJustPressed(unsigned int key)
{
    return key == last_key_press;
}

bool input::isKeyJustReleased(unsigned int key)
{
    return key == last_key_release;
}

bool input::isMouseButtonPressed(unsigned int button)
{
    return mouse_buttons[button % NUM_MOUSE_BUTTONS];
}

bool input::isMouseButtonJustPressed(unsigned int button)
{
    return button == last_mouse_button_press;
}

bool input::isMouseButtonJustReleased(unsigned int button)
{
    return button == last_mouse_button_release;
}

const float2 &input::getCursor()
{
    return cursor;
}

const float2 &input::getCursorDelta()
{
    return cursor_delta;
}

const float2 &input::getScrollDelta()
{
    return scroll_delta;
}
