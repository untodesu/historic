/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/gl/context.hpp>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

static void GLAPIENTRY debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const char *message, const void *)
{
    spdlog::debug(message);
}

void gl::preInitialize()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

#ifdef __APPLE__
    // As always MacOS with its Metal shits itself and dies
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
}

void gl::initialize()
{
    if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        spdlog::error("Unable to load OpenGL functions");
        std::terminate();
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, nullptr);

    // NVIDIA drivers really like to flood the
    // living shit out of the logs. Annoying!
    const GLuint nvidia_131185 = 131185;
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &nvidia_131185, GL_FALSE);
}
