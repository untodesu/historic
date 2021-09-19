/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/util/clock.hpp>
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/player_look.hpp>
#include <client/sys/player_move.hpp>
#include <client/sys/proj_view.hpp>
#include <client/sys/voxel_mesher.hpp>
#include <client/sys/voxel_renderer.hpp>
#include <client/client_app.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <ctime>
#include <client/gl/context.hpp>

static void glfwOnError(int code, const char *message)
{
    spdlog::error("GLFW ({}): {}", code, message);
}

void client_app::run()
{
    glfwSetErrorCallback(glfwOnError);
    if(!glfwInit()) {
        spdlog::error("glfwInit() failed.");
        std::terminate();
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    gl::setHints();

    // UNDONE: quake-ish CVar functions?
    GLFWwindow *window = glfwCreateWindow(1152, 648, "Client", nullptr, nullptr);
    if(!window) {
        spdlog::error("glfwCreateWindow() failed.");
        std::terminate();
    }

    glfwMakeContextCurrent(window);
    gl::init();

    input::init(window);
    screen::init(window);

    voxel_renderer::init();

    globals::registry.clear();

    // A test voxel #1
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.transparency = 0;
        vinfo.faces.push_back({ VOXEL_FACE_SIDES, "textures/vox_1.png" });
        vinfo.faces.push_back({ VOXEL_FACE_UP, "textures/vox_2.png" });
        vinfo.faces.push_back({ VOXEL_FACE_DN, "textures/vox_2.png" });
        globals::voxels.set(0xFF, vinfo);
    }

    // Player entity >_<
    {
        entt::entity player = globals::registry.create();
        globals::registry.emplace<ActiveCameraComponent>(player);
        globals::registry.emplace<LocalPlayerComponent>(player);
        globals::registry.emplace<CreatureComponent>(player);
        globals::registry.emplace<HeadComponent>(player);
        globals::registry.emplace<PlayerComponent>(player);

        CameraComponent &camera = globals::registry.emplace<CameraComponent>(player);
        camera.fov = glm::radians(90.0f);
        camera.z_far = 1024.0f;
        camera.z_near = 0.01f;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // A bunch of chunks with random stuff
    for(int i = -11; i < 12; i++) {
        for(int j = 0; j < 2; j++) {
            for(int k = -11; k < 12; k++) {
                const chunkpos_t cp = chunkpos_t(i, j, k);

                entt::entity chunk_ent = globals::registry.create();
                globals::registry.emplace<chunkpos_t>(chunk_ent, cp);
                globals::registry.emplace<NeedsVoxelMeshComponent>(chunk_ent);

                voxel_array_t *chunk = globals::chunks.findOrCreate(cp);
                //for(size_t u = 0; u < CHUNK_AREA; u++, chunk->at(std::rand() % CHUNK_VOLUME) = 0xFF);
                for(size_t u = 0; u < CHUNK_VOLUME; chunk->at(u++) = 0xFF);
            }
        }
    }

    globals::solid_textures.create(16, 16, MAX_VOXELS);
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        for(const VoxelFaceInfo &face : it->second.faces) {
            globals::solid_textures.push(face.texture);
        }
    }
    globals::solid_textures.submit();

    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Clock fps_clock, print_clock;
    float avg_frametime = 0.0f;
    globals::frame_count = 0;
    while(!glfwWindowShouldClose(window)) {
        globals::curtime = static_cast<float>(glfwGetTime());
        globals::frametime = fps_clock.restart();
        avg_frametime += globals::frametime;
        avg_frametime *= 0.5f;

        if(print_clock.elapsed() >= 1.0f) {
            spdlog::debug("Perf: {:.03f} ms ({:.02f} FPS)", avg_frametime * 1000.0f, 1.0f / avg_frametime);
            print_clock.restart();
        }

        // This should be an easier way to exit
        // than breaking my fingers to do alt+f4
        if(input::isKeyJustPressed(GLFW_KEY_ESCAPE))
            break;

        player_look::update();
        player_move::update();
        proj_view::update();

        voxel_mesher::update();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        voxel_renderer::update();

        glfwSwapBuffers(window);

        input::update();

        glfwPollEvents();
        globals::frame_count++;
    }

    spdlog::info("Client shutdown after {} frames, avg. dt: {}", globals::frame_count, avg_frametime);

    voxel_mesher::shutdown();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    globals::solid_textures.destroy();

    globals::registry.clear();

    voxel_renderer::shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
}
