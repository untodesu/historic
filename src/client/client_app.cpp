/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/clock.hpp>
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/player_look.hpp>
#include <client/sys/player_move.hpp>
#include <client/sys/proj_view.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/sys/voxel_renderer.hpp>
#include <client/client_app.hpp>
#include <client/globals.hpp>
#include <client/world.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <client/util.hpp>
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
#include <shared/voxel_def.hpp>

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

    // A test voxel #1
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.transparency = 0;
        vinfo.faces.push_back({ VOXEL_FACE_SIDES, "textures/test.jpg" });
        vinfo.faces.push_back({ VOXEL_FACE_UP, "textures/obama.png" });
        vinfo.faces.push_back({ VOXEL_FACE_DN, "textures/obama.png" });
        voxel_def::add(0xEE, vinfo);
    }

    // A test voxel #2
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.transparency = 0;
        vinfo.faces.push_back({ VOXEL_FACE_SOLID, "textures/obama.png" });
        voxel_def::add(0xFF, vinfo);
    }

    // A test voxel #3
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.transparency = 0;
        vinfo.faces.push_back({ VOXEL_FACE_SOLID, "textures/gman.jpeg" });
        voxel_def::add(0xAC, vinfo);
    }

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

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // A bunch of chunks with random stuff
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            entt::entity chunk = registry.create();
            ChunkComponent &comp = registry.emplace<ChunkComponent>(chunk);
            comp.position = chunkpos_t(i, 0, j);
            for(size_t u = 0; u < CHUNK_VOLUME; u++, comp.data[std::rand() % CHUNK_VOLUME] = 0xEE);
            registry.emplace<NeedsVoxelMeshComponent>(chunk);
        }
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Clock fps_clock, print_clock;
    float avg_frametime = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        const float frametime = fps_clock.restart();
        avg_frametime += frametime;
        avg_frametime *= 0.5f;

        if(print_clock.elapsed() >= 1.0f) {
            spdlog::debug("Perf: {:.03f} ms ({:.02f} FPS)", avg_frametime * 1000.0f, 1.0f / avg_frametime);
            print_clock.restart();
        }

        // This should be an easier way to exit
        // than breaking my fingers to do alt+f4
        if(input::isKeyJustPressed(GLFW_KEY_ESCAPE))
            break;

        player_look::update(frametime);
        player_move::update(frametime);
        proj_view::update();

        voxel_mesher::update();

        globals::render_device->prepare();

        globals::render_device->startRecording(commands);
        commands->setClearColor3f(0.3f, 0.0f, 0.3f);
        commands->clear(uvre::RT_COLOR_BUFFER | uvre::RT_DEPTH_BUFFER);
        globals::render_device->submit(commands);
        
        voxel_renderer::update();

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
