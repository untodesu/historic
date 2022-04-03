/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/client_app.hpp>
#include <client/config.hpp>
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/gl/context.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <common/cmdline.hpp>
#include <common/util/clock.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

static void onGlfwError(int code, const char *message)
{
    spdlog::error("glfw: {}", message);
}

void client_app::run()
{
    globals::config.read("config.toml");

    glfwSetErrorCallback(&onGlfwError);
    if(!glfwInit()) {
        spdlog::error("glfwInit() failed");
        std::terminate();
    }

    gl::preInitialize();

    // don't do that for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    globals::window = glfwCreateWindow(globals::config.window.width, globals::config.window.height, "Client", globals::config.window.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    if(!globals::window) {
        spdlog::error("GLFW: unable to create window");
        std::terminate();
    }

    glfwMakeContextCurrent(globals::window);

    gl::initialize();
    game::initialize();
    input::initialize();
    screen::initialize();

    glfwSwapInterval(globals::config.window.vsync ? 1 : 0);

    game::postInitialize();

    globals::curtime = 0.0f;
    globals::frame_count = 0;
    globals::frametime = 0.0f;
    globals::frametime_avg = 0.0f;
    globals::vertices_drawn = 0;

    util::Clock<std::chrono::high_resolution_clock> clock;
    util::Clock<std::chrono::high_resolution_clock> clock_avg;
    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = util::seconds<float>(clock.now().time_since_epoch());
        globals::frametime = util::seconds<float>(clock.reset());
        globals::vertices_drawn = 0;

        if(util::seconds<float>(clock_avg.elapsed()) > 0.0625f) {
            globals::frametime_avg += globals::frametime;
            globals::frametime_avg *= 0.5f;
            clock_avg.reset();
        }

        game::update();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        game::renderWorld();

        game::postRender();

        // Updating input right before
        // polling events - we reset some
        // values before events reset them.
        input::update();

        glfwSwapBuffers(globals::window);
        glfwPollEvents();

        globals::frame_count++;
    }

    spdlog::info("client shutdown after {} frames. avg dt: {:.03f} ms ({:.02f} FPS)", globals::frame_count, globals::frametime_avg * 1000.0f, 1.0f / globals::frametime_avg);

    game::shutdown();

    glfwDestroyWindow(globals::window);
    glfwTerminate();

    globals::config.write("config.toml");
}
