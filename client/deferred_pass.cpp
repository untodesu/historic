/*
 * deferred_pass.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/math/math.hpp>
#include <exception>
#include <client/render/gl/drawcmd.hpp>
#include <client/render/gl/pipeline.hpp>
#include <client/render/gl/sampler.hpp>
#include <client/render/gl/vertexarray.hpp>
#include <client/deferred_pass.hpp>
#include <client/util/include.hpp>
#include <client/render/gbuffer.hpp>
#include <client/systems/proj_view.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/shadow_manager.hpp>
#include <client/vertex.hpp>

struct alignas(16) Deferred_UBO0 final {
    float4 shadow_tweaks;
    float4 light_direction;
    float4 light_color;
    float4 ambient_color;
};

static struct {
    gl::Shader stages[2];
    gl::Pipeline pipeline;
    gl::Sampler samplers[2];
    gl::Buffer ubo_0;
    gl::Buffer ibo, vbo;
    gl::VertexArray vao;
    gl::DrawCommand cmd;
} deferred_ctx;

static const uint8_t quad_ids[6] = { 0, 1, 2, 2, 3, 0 };
static const QuadVertex2D quad_vts[4] = {
    QuadVertex2D { float2(-1.0f, -1.0f), float2(0.0f, 0.0f) },
    QuadVertex2D { float2(-1.0f,  1.0f), float2(0.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f,  1.0f), float2(1.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f, -1.0f), float2(1.0f, 0.0f) },
};

void deferred_pass::init()
{
    std::string source;

    deferred_ctx.stages[0].create();
    if(!util::include("shaders/deferred.vert.glsl", source) || !deferred_ctx.stages[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();

    deferred_ctx.stages[1].create();
    if(!util::include("shaders/deferred.frag.glsl", source) || !deferred_ctx.stages[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();
    
    deferred_ctx.pipeline.create();
    deferred_ctx.pipeline.stage(deferred_ctx.stages[0]);
    deferred_ctx.pipeline.stage(deferred_ctx.stages[1]);

    deferred_ctx.samplers[0].create();
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    deferred_ctx.samplers[0].parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    deferred_ctx.samplers[1].create();
    deferred_ctx.samplers[1].parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    deferred_ctx.samplers[1].parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    deferred_ctx.samplers[1].parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    deferred_ctx.samplers[1].parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    deferred_ctx.samplers[1].parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    deferred_ctx.ubo_0.create();
    deferred_ctx.ubo_0.storage(sizeof(Deferred_UBO0), nullptr, GL_DYNAMIC_STORAGE_BIT);

    deferred_ctx.ibo.create();
    deferred_ctx.ibo.storage(sizeof(quad_ids), quad_ids, 0);

    deferred_ctx.vbo.create();
    deferred_ctx.vbo.storage(sizeof(quad_vts), quad_vts, 0);

    deferred_ctx.vao.create();
    deferred_ctx.vao.setIndexBuffer(deferred_ctx.ibo);
    deferred_ctx.vao.setVertexBuffer(0, deferred_ctx.vbo, sizeof(QuadVertex2D));
    deferred_ctx.vao.enableAttribute(0, true);
    deferred_ctx.vao.setAttributeFormat(0, GL_FLOAT, 2, offsetof(QuadVertex2D, position), false);
    deferred_ctx.vao.setAttributeBinding(0, 0);
    deferred_ctx.vao.enableAttribute(1, true);
    deferred_ctx.vao.setAttributeFormat(1, GL_FLOAT, 2, offsetof(QuadVertex2D, texcoord), false);
    deferred_ctx.vao.setAttributeBinding(1, 0);

    deferred_ctx.cmd.create();
    deferred_ctx.cmd.set(GL_TRIANGLES, GL_UNSIGNED_BYTE, math::arraySize(quad_ids), 1, 0, 0, 0);
}

void deferred_pass::shutdown()
{
    deferred_ctx.cmd.destroy();
    deferred_ctx.vao.destroy();
    deferred_ctx.vbo.destroy();
    deferred_ctx.ibo.destroy();
    deferred_ctx.ubo_0.destroy();
    deferred_ctx.samplers[1].destroy();
    deferred_ctx.samplers[0].destroy();
    deferred_ctx.pipeline.destroy();
    deferred_ctx.stages[1].destroy();
    deferred_ctx.stages[0].destroy();
}

void deferred_pass::draw()
{
    int width, height;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    shadow_manager::getShadowMap().getSize(width, height);

    Deferred_UBO0 deferred_ubo_0 = {};
    deferred_ubo_0.shadow_tweaks = float4(width, height, 0.0f, 0.0f);
    deferred_ubo_0.light_direction = float4(shadow_manager::getLightDirection(), 0.0f);
    deferred_ubo_0.light_color = float4(shadow_manager::getLightColor(), 0.0f);
    deferred_ubo_0.ambient_color = float4(0.25, 0.25, 0.25, 0.0f);
    deferred_ctx.ubo_0.write(0, sizeof(Deferred_UBO0), &deferred_ubo_0);

    globals::solid_gbuffer.getAlbedo().bind(0);
    globals::solid_gbuffer.getNormal().bind(1);
    globals::solid_gbuffer.getPosition().bind(2);
    globals::solid_gbuffer.getShadowProjCoord().bind(3);
    shadow_manager::getShadowMap().getShadow().bind(4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, deferred_ctx.ubo_0.get());
    deferred_ctx.samplers[0].bind(0);
    deferred_ctx.samplers[0].bind(1);
    deferred_ctx.samplers[0].bind(2);
    deferred_ctx.samplers[0].bind(3);
    deferred_ctx.samplers[1].bind(4);
    deferred_ctx.pipeline.bind();
    deferred_ctx.vao.bind();
    deferred_ctx.cmd.invoke();
}
