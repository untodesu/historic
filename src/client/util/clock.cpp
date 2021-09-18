/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/util/clock.hpp>
#include <GLFW/glfw3.h>

Clock::Clock()
    : start(static_cast<float>(glfwGetTime()))
{

}

const float Clock::elapsed() const
{
    return static_cast<float>(glfwGetTime()) - start;
}

const float Clock::restart()
{
    float current = static_cast<float>(glfwGetTime());
    float delta = current - start;
    start = current;
    return delta;
}
