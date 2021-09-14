/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/screen.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

static float2_t screen_size = FLOAT2_IDENTITY;
static float aspect_ratio = 1.0f;

static void onFramebufferSize(GLFWwindow *, int width, int height)
{
    screen_size = float2_t(width, height);
    aspect_ratio = (screen_size.x > screen_size.y) ? (screen_size.x / screen_size.y) : (screen_size.y / screen_size.x);
}

void screen::init(GLFWwindow *window)
{
    spdlog::debug("Hooking screen events");
    
    int width, height;
    glfwSetFramebufferSizeCallback(window, onFramebufferSize);
    glfwGetFramebufferSize(window, &width, &height);
    onFramebufferSize(window, width, height);
}

const float2_t &screen::getSize()
{
    return screen_size;
}

float screen::getAspectRatio()
{
    return aspect_ratio;
}
