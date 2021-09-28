/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/gl/context.hpp>
#include <game/client/util/clock.hpp>
#include <game/client/client_app.hpp>
#include <game/client/debug_overlay.hpp>
#include <game/client/globals.hpp>
#include <game/client/input.hpp>
#include <game/client/network.hpp>
#include <game/client/screen.hpp>
#include <game/shared/protocol/client/handshake.hpp>
#include <game/shared/protocol/util.hpp>
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
    network::init();

    if(!network::connect("localhost", 24000))
        std::terminate();

    const std::vector<uint8_t> hsbuf = protocol::serialize(protocol::Handshake(42));
    network::send(hsbuf.size(), hsbuf.data());

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

    input::init();
    screen::init();

    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(globals::window, false);
    ImGui_ImplOpenGL3_Init("#version 460");

    Clock fps_clock;
    globals::frame_count = 0;
    while(!glfwWindowShouldClose(globals::window)) {
        globals::vertices_drawn = 0;
        globals::curtime = static_cast<float>(glfwGetTime());
        globals::frametime = fps_clock.restart();
        globals::avg_frametime += globals::frametime;
        globals::avg_frametime *= 0.5f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        debug_overlay::draw();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(globals::window);

        input::update();

        glfwPollEvents();
        globals::frame_count++;
    }

    spdlog::info("Client shutdown after {} frames, avg. dt: {}", globals::frame_count, globals::avg_frametime);

    network::shutdown();

    glfwDestroyWindow(globals::window);
    glfwTerminate();
}
