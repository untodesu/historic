/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/player_look.hpp>
#include <client/sys/player_move.hpp>
#include <client/sys/proj_view.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/sys/voxel_renderer.hpp>
#include <client/client_app.hpp>
#include <client/client_globals.hpp>
#include <client/client_world.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/res.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>
#include <cstdlib>
#include <ctime>

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
        glfwWindowHint(GLFW_DEPTH_BITS, 24);

#ifdef __APPLE__
        // As always MacOS with its Metal shits itself and dies
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    }

    // UNDONE: quake-ish CVar functions?
    GLFWwindow *window = glfwCreateWindow(1152, 648, "Client", nullptr, nullptr);
    if(!window) {
        spdlog::error("glfwCreateWindow() failed.");
        std::terminate();
    }

    input::init(window);
    screen::init(window);

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

    globals::render_device = uvre::createDevice(device_info);
    if(!globals::render_device) {
        spdlog::error("uvre::createDevice() failed.");
        std::terminate();
    }

    // We do a little trolling
    globals::render_device->vsync(false);

    uvre::ICommandList *commands = globals::render_device->createCommandList();

    voxel_renderer::init();

    client_world::init();
    entt::registry &registry = client_world::registry();

    // Player entity >_<
    {
        entt::entity player = registry.create();
        registry.emplace<ActiveCameraComponent>(player);
        registry.emplace<LocalPlayerComponent>(player);
        registry.emplace<CreatureComponent>(player);
        registry.emplace<HeadComponent>(player);
        registry.emplace<PlayerComponent>(player);

        CameraComponent &camera = registry.emplace<CameraComponent>(player);
        camera.fov = glm::radians(90.0f);
        camera.z_far = 1024.0f;
        camera.z_near = 0.01f;
    }

    // A chunk!
    {
        entt::entity chunk = registry.create();

        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        ChunkComponent &comp = registry.emplace<ChunkComponent>(chunk);
        comp.position = chunkpos_t(0, 0, 0);
        for(size_t i = 0; i < CHUNK_AREA; i++)
            comp.data[std::rand() % static_cast<int>(CHUNK_VOLUME)] = 0xFF;

        registry.emplace<NeedsVoxelMeshComponent>(chunk);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float lasttime = static_cast<float>(glfwGetTime());

    while(!glfwWindowShouldClose(window)) {
        float curtime = static_cast<float>(glfwGetTime());
        float frametime = curtime - lasttime;
        lasttime = curtime;

        // This should be an easier way to exit
        // than breaking my fingers to do alt+f4
        if(input::isKeyJustPressed(GLFW_KEY_ESCAPE))
            break;

        player_look::update(frametime);
        player_move::update(frametime);
        proj_view::update();

        voxel_mesher::update(registry);

        globals::render_device->prepare();

        globals::render_device->startRecording(commands);
        commands->setClearColor3f(0.3f, 0.0f, 0.3f);
        commands->clear(uvre::RT_COLOR_BUFFER | uvre::RT_DEPTH_BUFFER);
        globals::render_device->submit(commands);
        
        voxel_renderer::update(registry);

        globals::render_device->present();

        input::update();
        glfwPollEvents();
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    client_world::shutdown();

    res::cleanup<uvre::Texture>(res::PRECACHE);

    voxel_renderer::shutdown();

    globals::render_device->destroyCommandList(commands);

    uvre::destroyDevice(globals::render_device);
    glfwDestroyWindow(window);
    glfwTerminate();
}
