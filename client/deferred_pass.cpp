/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/deferred_pass.hpp>
#include <client/gbuffer.hpp>
#include <client/gl/drawcmd.hpp>
#include <client/gl/pipeline.hpp>
#include <client/gl/sampler.hpp>
#include <client/gl/shader.hpp>
#include <client/gl/vertex_array.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <common/util/include.hpp>

static struct {
    gl::Shader shaders[2];
    gl::Pipeline pipeline;
    gl::Sampler samplers[1];
    gl::Buffer ndc_ibo;
    gl::VertexArray ndc_vao;
    gl::DrawCommand ndc_cmd;
} deferred_ctx;

static const GLubyte ndc_indices[6] = {
    0, 1, 2,
    2, 3, 0,
};

void deferred_pass::initialize()
{
    std::string glsl_source;

    deferred_ctx.shaders[0].create();
    deferred_ctx.shaders[1].create();

    if(!util::include("shaders/deferred.vert", glsl_source) || !deferred_ctx.shaders[0].glsl(GL_VERTEX_SHADER, glsl_source))
        std::terminate();
    if(!util::include("shaders/deferred.frag", glsl_source) || !deferred_ctx.shaders[1].glsl(GL_FRAGMENT_SHADER, glsl_source))
        std::terminate();

    deferred_ctx.pipeline.create();
    deferred_ctx.pipeline.stage(deferred_ctx.shaders[0]);
    deferred_ctx.pipeline.stage(deferred_ctx.shaders[1]);

    deferred_ctx.samplers[0].create();
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    deferred_ctx.ndc_ibo.create();
    deferred_ctx.ndc_ibo.storage(sizeof(ndc_indices), ndc_indices, 0);

    deferred_ctx.ndc_vao.create();
    deferred_ctx.ndc_vao.setIndexBuffer(deferred_ctx.ndc_ibo);

    deferred_ctx.ndc_cmd.create();
    deferred_ctx.ndc_cmd.set(GL_TRIANGLES, GL_UNSIGNED_BYTE, math::arraySize(ndc_indices), 1, 0, 0, 0);
}

void deferred_pass::shutdown()
{
    deferred_ctx.ndc_cmd.destroy();
    deferred_ctx.ndc_vao.destroy();
    deferred_ctx.ndc_ibo.destroy();
    deferred_ctx.samplers[0].destroy();
    deferred_ctx.pipeline.destroy();
    deferred_ctx.shaders[1].destroy();
    deferred_ctx.shaders[0].destroy();
}

void deferred_pass::renderWorld()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const vector2i_t size = screen::size2i();
    glViewport(0, 0, size.x, size.y);

    // TODO: postproc_pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    globals::main_gbuffer.getAlbedoAttachment().bind(0);
    globals::main_gbuffer.getNormalAttachment().bind(1);

    deferred_ctx.samplers[0].bind(0);
    deferred_ctx.samplers[0].bind(1);
    deferred_ctx.pipeline.bind();

    deferred_ctx.ndc_vao.bind();
    deferred_ctx.ndc_cmd.invoke();
}
