/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <enet/enet.h>
#include <exception>
#include <client/gl/context.hpp>
#include <client/client_app.hpp>
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <shared/util/clock.hpp>
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

    ChronoClock<std::chrono::high_resolution_clock> frametime_clock;
    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = util::seconds<float>(frametime_clock.now().time_since_epoch());
        globals::frametime = util::seconds<float>(frametime_clock.restart());
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
        game::drawImgui();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        game::postDraw();

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
