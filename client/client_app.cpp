/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/client_app.hpp>
#include <client/config.hpp>
#include <client/globals.hpp>
#include <client/gl/context.hpp>
#include <common/cmdline.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

// TEST
#include <common/types.hpp>
#include <client/gl/drawcmd.hpp>
#include <client/gl/pipeline.hpp>
#include <client/gl/vertex_array.hpp>

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

    struct vertex final {
        vector2f_t position;
        vector2f_t uv;
    };

    const vertex vtxs[3] = {
        vertex { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
        vertex { {  0.0f,  0.5f }, { 0.5f, 1.0f } },
        vertex { {  0.5f, -0.5f }, { 1.0f, 0.0f } }
    };

    gl::Buffer vbo;
    vbo.create();
    vbo.storage(sizeof(vtxs), vtxs, 0);

    gl::VertexArray vao;
    vao.create();
    vao.setVertexBuffer(0, vbo, sizeof(vertex));
    vao.enableAttribute(0, true);
    vao.enableAttribute(1, true);
    vao.setAttributeFormat(0, GL_FLOAT, 2, offsetof(vertex, position), false);
    vao.setAttributeFormat(1, GL_FLOAT, 2, offsetof(vertex, uv), false);
    vao.setAttributeBinding(0, 0);
    vao.setAttributeBinding(1, 0);

    gl::DrawCommand cmd(GL_TRIANGLES, 3, 1, 0, 0);

    gl::Shader shaders[2];

    shaders[0].create();
    shaders[0].glsl(GL_VERTEX_SHADER, R"(
        #version 460 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;
        layout(location = 0) out vec2 out_uv;
        out gl_PerVertex { vec4 gl_Position; };
        void main()
        {
            out_uv = uv;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )");

    shaders[1].create();
    shaders[1].glsl(GL_FRAGMENT_SHADER, R"(
        #version 460 core
        layout(location = 0) in vec2 uv;
        layout(location = 0) out vec4 color;
        void main()
        {
            color = vec4(uv, 1.0, 1.0);
        }
    )");

    gl::Pipeline pipeline;
    pipeline.create();
    pipeline.stage(shaders[0]);
    pipeline.stage(shaders[1]);

    while(!glfwWindowShouldClose(globals::window)) {
        glUseProgram(0);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        pipeline.bind();
        glBindVertexArray(vao.get());
        cmd.invoke();

        glfwSwapBuffers(globals::window);

        glfwPollEvents();
    }

    pipeline.destroy();
    shaders[1].destroy();
    shaders[0].destroy();
    vao.destroy();
    vbo.destroy();

    glfwDestroyWindow(globals::window);
    glfwTerminate();

    globals::config.write("config.toml");
}
