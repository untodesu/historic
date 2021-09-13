/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/camera.hpp>
#include <client/camera_controller.hpp>
#include <client/client_app.hpp>
#include <client/generic_mesh.hpp>
#include <client/generic_renderer.hpp>
#include <client/input.hpp>
#include <client/vertex.hpp>
#include <entt/entt.hpp>
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

    input::init(window);

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

    generic_renderer::init(device);

    entt::registry registry;

    // Camera entity >_<
    entt::entity cam_ent = registry.create();
    registry.emplace<ActiveCameraComponent>(cam_ent);
    registry.emplace<ControlledCameraComponent>(cam_ent);
    CameraComponent &cam_comp = registry.emplace<CameraComponent>(cam_ent);
    cam_comp.z_near = 0.01f;
    cam_comp.z_far = 1024.0f;
    cam_comp.aspect = 800.0f / 600.0f;
    cam_comp.fov = glm::radians(90.0f);
    cam_comp.offset = float3_t(0.0f, 0.0f, 0.25f);

    // Quad vertices
    const Vertex vertices[4] = {
        Vertex { float3_t(-0.5f, -0.5f, 0.0f), float2_t(0.0f, 1.0f) },
        Vertex { float3_t(-0.5f,  0.5f, 0.0f), float2_t(0.0f, 0.0f) },
        Vertex { float3_t( 0.5f,  0.5f, 0.0f), float2_t(1.0f, 0.0f) },
        Vertex { float3_t( 0.5f, -0.5f, 0.0f), float2_t(1.0f, 1.0f) }
    };

    // Quad indices
    const uvre::Index16 indices[6] = { 0, 1, 2, 2, 3, 0 };

    uvre::BufferInfo ibo_info = {};
    ibo_info.type = uvre::BufferType::INDEX_BUFFER;
    ibo_info.size = sizeof(indices);
    ibo_info.data = indices;

    uvre::BufferInfo vbo_info = {};
    vbo_info.type = uvre::BufferType::VERTEX_BUFFER;
    vbo_info.size = sizeof(vertices);
    vbo_info.data = vertices;

    entt::entity quad_ent = registry.create();
    GenericMeshComponent &quad_mesh = registry.emplace<GenericMeshComponent>(quad_ent);
    quad_mesh.ibo = device->createBuffer(ibo_info);
    quad_mesh.vbo = device->createBuffer(vbo_info);
    quad_mesh.nv = 6;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float lasttime = static_cast<float>(glfwGetTime());

    while(!glfwWindowShouldClose(window)) {
        float curtime = static_cast<float>(glfwGetTime());
        float frametime = curtime - lasttime;
        lasttime = curtime;

        camera_controller::update(registry, frametime);

        device->prepare();

        device->startRecording(commands);
        commands->clearColor3f(0.3f, 0.0f, 0.3f);
        commands->clear(uvre::RT_COLOR_BUFFER);
        device->submit(commands);
        
        generic_renderer::update(registry);

        device->present();

        input::update();
        glfwPollEvents();
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    generic_renderer::shutdown();

    device->destroyCommandList(commands);

    uvre::destroyDevice(device);
    glfwDestroyWindow(window);
    glfwTerminate();
}
