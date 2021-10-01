/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/filesystem.hpp>
#include <common/math/math.hpp>
#include <exception>
#include <game/client/gl/drawcmd.hpp>
#include <game/client/gl/pipeline.hpp>
#include <game/client/gl/sampler.hpp>
#include <game/client/gl/vertexarray.hpp>
#include <game/client/sys/proj_view.hpp>
#include <game/client/composite.hpp>
#include <game/client/gbuffer.hpp>
#include <game/client/globals.hpp>
#include <game/client/screen.hpp>
#include <game/client/shadow_manager.hpp>
#include <game/client/vertex.hpp>

struct alignas(16) UBufferData_Composite final {
    float4 tweaks;
    float4 light_direction;
    float4 light_color;
    float4 ambient;
};

static gl::Buffer composite_ibo, composite_vbo;
static gl::VertexArray composite_vao;
static gl::DrawCommand composite_drawcmd;
static gl::Buffer composite_ubuffer;
static gl::Shader composite_shaders[2];
static gl::Pipeline composite_pipeline;
static gl::Sampler composite_samplers[2];

static const uint8_t quad_inds[6] = { 0, 1, 2, 2, 3, 0 };
static const QuadVertex2D quad_verts[4] = {
    QuadVertex2D { float2(-1.0f, -1.0f), float2(0.0f, 0.0f) },
    QuadVertex2D { float2(-1.0f,  1.0f), float2(0.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f,  1.0f), float2(1.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f, -1.0f), float2(1.0f, 0.0f) },
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
    composite_vao.setVertexBuffer(0, composite_vbo, sizeof(QuadVertex2D));
    composite_vao.enableAttribute(0, true);
    composite_vao.setAttributeFormat(0, GL_FLOAT, 2, offsetof(QuadVertex2D, position), false);
    composite_vao.setAttributeBinding(0, 0);
    composite_vao.enableAttribute(1, true);
    composite_vao.setAttributeFormat(1, GL_FLOAT, 2, offsetof(QuadVertex2D, texcoord), false);
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

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int sw, sh;
    shadow_manager::getShadowMap().getSize(sw, sh);

    UBufferData_Composite composite_ubuffer_data = {};
    composite_ubuffer_data.tweaks = float4(sw, sh, 0.0f, 0.0f);
    composite_ubuffer_data.light_direction = float4(shadow_manager::getLightDirection(), 0.0f);
    composite_ubuffer_data.light_color = float4(shadow_manager::getLightColor(), 0.0f);
    composite_ubuffer_data.ambient = float4(0.25f, 0.25f, 0.25f, 0.0f);
    composite_ubuffer.write(0, sizeof(UBufferData_Composite), &composite_ubuffer_data);

    globals::solid_gbuffer.getAlbedo().bind(0);
    globals::solid_gbuffer.getNormal().bind(1);
    globals::solid_gbuffer.getPosition().bind(2);
    globals::solid_gbuffer.getShadowProjCoord().bind(3);
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
