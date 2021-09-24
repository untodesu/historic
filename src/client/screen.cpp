/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <glad/gl.h>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

static float2 screen_size = FLOAT2_IDENTITY;
static int screen_width = 1;
static int screen_height = 1;
static float aspect_ratio = 1.0f;

static void onFramebufferSize(GLFWwindow *, int width, int height)
{
    cl_globals::solid_gbuffer.init(width, height);

    screen_size = float2(width, height);
    screen_width = width;
    screen_height = height;
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

float screen::getAspectRatio()
{
    return aspect_ratio;
}

const float2 &screen::getSize()
{
    return screen_size;
}

void screen::getSize(int &width, int &height)
{
    width = screen_width;
    height = screen_height;
}
