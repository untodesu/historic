/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <enet/enet.h>
#include <exception>
#include <game/client/gl/context.hpp>
#include <game/client/util/clock.hpp>
#include <game/client/client_app.hpp>
#include <game/client/debug_overlay.hpp>
#include <game/client/game.hpp>
#include <game/client/globals.hpp>
#include <game/client/input.hpp>
#include <game/client/screen.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

static void glfwOnError(int code, const char *message)
{
    spdlog::error("GLFW ({}): {}", code, message);
}

void client_app::run()
{
    globals::host = enet_host_create(nullptr, 1, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a client host object.");
        std::terminate();
    }

    glfwSetErrorCallback(glfwOnError);
    if(!glfwInit()) {
        spdlog::error("glfwInit() failed.");
        std::terminate();
    }

    gl::setHints();

    globals::window = glfwCreateWindow(1152, 648, "Client", nullptr, nullptr);
    if(!globals::window) {
        spdlog::error("glfwCreateWindow() failed.");
        std::terminate();
    }

    glfwMakeContextCurrent(globals::window);
    gl::init();

    game::init();

    input::init();
    screen::init();
    
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(globals::window, false);
    ImGui_ImplOpenGL3_Init("#version 460");

    game::postInit();

    globals::curtime = 0.0f;
    globals::frametime = 0.0f;
    globals::avg_frametime = 0.0f;
    globals::frame_count = 0;

    Clock frametime_clock;
    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = static_cast<float>(glfwGetTime());
        globals::frametime = frametime_clock.restart();
        globals::avg_frametime += globals::frametime;
        globals::avg_frametime *= 0.5f;
        globals::vertices_drawn = 0;

        game::update();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        game::draw();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        game::imgui();
        debug_overlay::draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(globals::window);

        input::update();
        glfwPollEvents();

        globals::frame_count++;
    }

    game::shutdown();
    spdlog::info("Client shutdown after {} frames. Avg. dt: {:.03f} ms ({:.02f} FPS)", globals::frame_count, globals::avg_frametime * 1000.0f, 1.0f / globals::avg_frametime);

    glfwDestroyWindow(globals::window);
    glfwTerminate();

    enet_host_destroy(globals::host);
    globals::host = nullptr;
}