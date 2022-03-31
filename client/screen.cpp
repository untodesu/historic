/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

static vector2f_t screen_size_v2f = VECTOR2F_ZERO;
static vector2i_t screen_size_v2i = { 0, 0 };
static float screen_aspect = 1.0f;

static void onFramebufferSize(GLFWwindow *, int width, int height)
{
    game::mode(width, height);
    screen_size_v2f = vector2f_t(width, height);
    screen_aspect = (screen_size_v2f.x > screen_size_v2f.y)
        ? (screen_size_v2f.x / screen_size_v2f.y)
        : (screen_size_v2f.y / screen_size_v2f.x);
    screen_size_v2i = vector2i_t(width, height);
}

void screen::initialize()
{
    spdlog::debug("screen: taking over framebuffer size callback");

    int width, height;
    glfwSetFramebufferSizeCallback(globals::window, &onFramebufferSize);
    glfwGetFramebufferSize(globals::window, &width, &height);
    onFramebufferSize(globals::window, width, height);
}

float screen::aspect()
{
    return screen_aspect;
}

const vector2f_t &screen::size2f()
{
    return screen_size_v2f;
}

const vector2i_t &screen::size2i()
{
    return screen_size_v2i;
}
