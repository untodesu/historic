/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/client_app.hpp>
#include <exception>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>

static void glfwOnError(int code, const char *message)
{
    spdlog::error("GLFW ({}): {}", code, message);
}

static void *uvreglGetProcAddr(void *, const char *procname)
{
    return reinterpret_cast<void *>(glfwGetProcAddress(procname));
}

static void uvreglMakeContextCurrent(void *arg)
{
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow *>(arg));
}

static void uvreglSetSwapInterval(void *, int interval)
{
    glfwSwapInterval(interval);
}

static void uvreglSwapBuffers(void *arg)
{
    glfwSwapBuffers(reinterpret_cast<GLFWwindow *>(arg));
}

static void uvreOnMessage(const char *message)
{
    spdlog::info(message);
}

void client_app::run()
{
    glfwSetErrorCallback(glfwOnError);
    if(!glfwInit()) {
        spdlog::error("glfwInit() failed.");
        std::terminate();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    uvre::BackendInfo backend_info;
    uvre::pollBackendInfo(backend_info);

    // OpenGL-related GLFW hints
    if(backend_info.family == uvre::BackendFamily::OPENGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE, backend_info.gl.core_profile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_ANY_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, backend_info.gl.version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, backend_info.gl.version_minor);

#ifdef __APPLE__
        // As always MacOS with its Metal shits itself and dies
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    }

    // UNDONE: quake-ish CVar functions?
    GLFWwindow *window = glfwCreateWindow(640, 480, "Client", nullptr, nullptr);
    if(!window) {
        spdlog::error("glfwCreateWindow() failed.");
        std::terminate();
    }

    uvre::DeviceInfo device_info = {};
    device_info.onMessage = uvreOnMessage;

    // OpenGL-related device information
    if(backend_info.family == uvre::BackendFamily::OPENGL) {
        device_info.gl.getProcAddr = uvreglGetProcAddr;
        device_info.gl.makeContextCurrent = uvreglMakeContextCurrent;
        device_info.gl.setSwapInterval = uvreglSetSwapInterval;
        device_info.gl.swapBuffers = uvreglSwapBuffers;
        device_info.gl.user_data = window;
    }

    uvre::IRenderDevice *device = uvre::createDevice(device_info);
    if(!device) {
        spdlog::error("uvre::createDevice() failed.");
        std::terminate();
    }

    uvre::ICommandList *commands = device->createCommandList();

    while(!glfwWindowShouldClose(window)) {
        device->prepare();
        device->startRecording(commands);
        commands->clearColor3f(0.3f, 0.0f, 0.3f);
        commands->clear(uvre::RT_COLOR_BUFFER);
        device->submit(commands);
        device->present();
        glfwPollEvents();
    }

    device->destroyCommandList(commands);

    uvre::destroyDevice(device);
    glfwDestroyWindow(window);
    glfwTerminate();
}
