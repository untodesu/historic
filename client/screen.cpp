/*
 * screen.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <glad/gl.h>
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <GLFW/glfw3.h>
#include <common/math/const.hpp>
#include <spdlog/spdlog.h>

static float2 screen_size = FLOAT2_IDENTITY;
static int screen_width = 1;
static int screen_height = 1;
static float aspect_ratio = 1.0f;

static void onFramebufferSize(GLFWwindow *, int width, int height)
{
    game::modeChange(width, height);
    screen_size = float2(width, height);
    screen_width = width;
    screen_height = height;
    aspect_ratio = (screen_size.x > screen_size.y) ? (screen_size.x / screen_size.y) : (screen_size.y / screen_size.x);
}

void screen::init()
{
    spdlog::debug("Hooking screen events");
    
    int width, height;
    glfwSetFramebufferSizeCallback(globals::window, onFramebufferSize);
    glfwGetFramebufferSize(globals::window, &width, &height);
    onFramebufferSize(globals::window, width, height);
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
