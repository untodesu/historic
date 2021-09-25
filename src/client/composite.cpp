/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/gl/drawcmd.hpp>
#include <client/gl/pipeline.hpp>
#include <client/gl/sampler.hpp>
#include <client/gl/vertexarray.hpp>
#include <client/sys/proj_view.hpp>
#include <client/composite.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/shadow_manager.hpp>
#include <exception>
#include <filesystem.hpp>
#include <math/math.hpp>

struct alignas(16) UBufferData_Composite final {
    float4 tweaks;
    float4 light_direction;
    float4 light_color;
    float4 ambient;
};

constexpr const size_t ss = sizeof(UBufferData_Composite);

struct QuadVertex final {
    float2 position;
    float2 texcoord;
};

static gl::Buffer composite_ibo, composite_vbo;
static gl::VertexArray composite_vao;
static gl::DrawCommand composite_drawcmd;
static gl::Buffer composite_ubuffer;
static gl::Shader composite_shaders[2];
static gl::Pipeline composite_pipeline;
static gl::Sampler composite_samplers[2];

static const uint8_t quad_inds[6] = { 0, 1, 2, 2, 3, 0 };
static const QuadVertex quad_verts[4] = {
    QuadVertex { float2(-1.0f, -1.0f), float2(0.0f, 0.0f) },
    QuadVertex { float2(-1.0f,  1.0f), float2(0.0f, 1.0f) },
    QuadVertex { float2( 1.0f,  1.0f), float2(1.0f, 1.0f) },
    QuadVertex { float2( 1.0f, -1.0f), float2(1.0f, 0.0f) },
};

void composite::init()
{
    std::string source;

    composite_ibo.create();
    composite_ibo.storage(sizeof(quad_inds), quad_inds, 0);

    composite_vbo.create();
    composite_vbo.storage(sizeof(quad_verts), quad_verts, 0);

    composite_vao.create();
    composite_vao.setIndexBuffer(composite_ibo);
    composite_vao.setVertexBuffer(0, composite_vbo, sizeof(QuadVertex));
    composite_vao.enableAttribute(0, true);
    composite_vao.setAttributeFormat(0, GL_FLOAT, 2, offsetof(QuadVertex, position), false);
    composite_vao.setAttributeBinding(0, 0);
    composite_vao.enableAttribute(1, true);
    composite_vao.setAttributeFormat(1, GL_FLOAT, 2, offsetof(QuadVertex, texcoord), false);
    composite_vao.setAttributeBinding(1, 0);

    composite_drawcmd.create();
    composite_drawcmd.set(GL_TRIANGLES, GL_UNSIGNED_BYTE, math::arraySize(quad_inds), 1, 0, 0, 0);

    composite_ubuffer.create();
    composite_ubuffer.storage(sizeof(UBufferData_Composite), nullptr, GL_DYNAMIC_STORAGE_BIT);

    composite_shaders[0].create();
    if(!fs::readText("shaders/composite.vert.glsl", source) || !composite_shaders[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();

    composite_shaders[1].create();
    if(!fs::readText("shaders/composite.frag.glsl", source) || !composite_shaders[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();

    composite_pipeline.create();
    composite_pipeline.stage(composite_shaders[0]);
    composite_pipeline.stage(composite_shaders[1]);

    composite_samplers[0].create();
    composite_samplers[0].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    composite_samplers[0].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    composite_samplers[0].parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    composite_samplers[0].parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    composite_samplers[1].create();
    composite_samplers[1].parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    composite_samplers[1].parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    composite_samplers[1].parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    composite_samplers[1].parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    composite_samplers[1].parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
}

void composite::shutdown()
{
    composite_samplers[1].destroy();
    composite_samplers[0].destroy();
    composite_pipeline.destroy();
    composite_shaders[1].destroy();
    composite_shaders[0].destroy();
    composite_ubuffer.destroy();
    composite_drawcmd.destroy();
    composite_vao.destroy();
    composite_vbo.destroy();
    composite_ibo.destroy();
}

void composite::draw()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int width, height;
    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int sw, sh;
    shadow_manager::getShadowMap().getSize(sw, sh);

    UBufferData_Composite composite_ubuffer_data = {};
    composite_ubuffer_data.tweaks = float4(sw, sh, 0.0f, 0.0f);
    composite_ubuffer_data.light_direction = float4(shadow_manager::getLightDirection(), 0.0f);
    composite_ubuffer_data.light_color = float4(shadow_manager::getLightColor(), 0.0f);
    composite_ubuffer_data.ambient = float4(0.25f, 0.25f, 0.25f, 0.0f);
    composite_ubuffer.write(0, sizeof(UBufferData_Composite), &composite_ubuffer_data);

    cl_globals::solid_gbuffer.getAlbedo().bind(0);
    cl_globals::solid_gbuffer.getNormal().bind(1);
    cl_globals::solid_gbuffer.getPosition().bind(2);
    cl_globals::solid_gbuffer.getShadowProjCoord().bind(3);
    shadow_manager::getShadowMap().getShadow().bind(4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, composite_ubuffer.get());
    composite_samplers[0].bind(0);
    composite_samplers[0].bind(1);
    composite_samplers[0].bind(2);
    composite_samplers[0].bind(3);
    composite_samplers[1].bind(4);
    composite_pipeline.bind();
    composite_vao.bind();
    composite_drawcmd.invoke();
}
