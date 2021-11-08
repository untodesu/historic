/*
 * client_app.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <exception>
#include <client/render/gl/context.hpp>
#include <client/client_app.hpp>
#include <client/fontlib.hpp>
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/network.hpp>
#include <client/screen.hpp>
#include <shared/script/cvars.hpp>
#include <shared/script/script_engine.hpp>
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

static duk_ret_t onAppExit(duk_context *)
{
    glfwSetWindowShouldClose(globals::window, GLFW_TRUE);
    return 0;
}

static duk_ret_t onAppSetTitle(duk_context *ctx)
{
    glfwSetWindowTitle(globals::window, duk_safe_to_string(ctx, 0));
    return 0;
}

void client_app::run()
{
    globals::script.build("App")
        .constant("CLIENT", 1)
        .constant("SERVER", 0)
        .function("exit", &onAppExit, 0)
        .function("setTitle", &onAppSetTitle, 1)
        .submit();

    network::preInit();
    game::preInit();

    glfwSetErrorCallback(&glfwOnError);
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
    network::init();
    game::init();
    input::init();
    screen::init();
    
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(globals::window, false);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    fontlib::init();

    const stdfs::path js_subdir = stdfs::path("scripts") / stdfs::path("client");
    const stdfs::path init_path = js_subdir / stdfs::path("init.js");
    const stdfs::path user_path = js_subdir / stdfs::path("user.js");
    stdfs::create_directories(fs::getWritePath(js_subdir));

    cvars::setInitMode(true);
    globals::script.exec(init_path);
    globals::script.exec(user_path);
    cvars::setInitMode(false);

    game::postInit();

    globals::curtime = 0.0f;
    globals::frametime = 0.0f;
    globals::avg_frametime = 0.0f;
    globals::frame_count = 0;

    ChronoClock<std::chrono::high_resolution_clock> clock;
    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = util::seconds<float>(clock.now().time_since_epoch());
        globals::frametime = util::seconds<float>(clock.restart());
        globals::avg_frametime += globals::frametime;
        globals::avg_frametime *= 0.5f;
        globals::vertices_drawn = 0;
        globals::ui_grabs_input = false;

        network::update();
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

        input::update();

        glfwSwapBuffers(globals::window);
        glfwPollEvents();

        globals::frame_count++;
    }

    spdlog::info("Client shutdown after {} frames. Avg. dt: {:.03f} ms ({:.02f} FPS)", globals::frame_count, globals::avg_frametime * 1000.0f, 1.0f / globals::avg_frametime);

    fontlib::shutdown();
    game::shutdown();
    network::shutdown();

    spdlog::debug("Writing {}", init_path.string());
    fs::writeText(init_path, cvars::dump());

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(globals::window);
    glfwTerminate();
}
