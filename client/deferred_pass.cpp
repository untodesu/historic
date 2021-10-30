/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/filesystem.hpp>
#include <common/math/math.hpp>
#include <exception>
#include <client/render/gl/drawcmd.hpp>
#include <client/render/gl/pipeline.hpp>
#include <client/render/gl/sampler.hpp>
#include <client/render/gl/vertexarray.hpp>
#include <client/deferred_pass.hpp>
#include <client/render/gbuffer.hpp>
#include <client/systems/proj_view.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/shadow_manager.hpp>
#include <client/vertex.hpp>

struct alignas(16) UBufferData_DeferredPass final {
    float4 tweaks;
    float4 light_direction;
    float4 light_color;
    float4 ambient;
};

static gl::Buffer deferred_ibo, deferred_vbo;
static gl::VertexArray deferred_vao;
static gl::DrawCommand deferred_drawcmd;
static gl::Buffer deferred_ubuffer;
static gl::Shader deferred_shaders[2];
static gl::Pipeline deferred_pipeline;
static gl::Sampler deferred_samplers[2];

static const uint8_t quad_inds[6] = { 0, 1, 2, 2, 3, 0 };
static const QuadVertex2D quad_verts[4] = {
    QuadVertex2D { float2(-1.0f, -1.0f), float2(0.0f, 0.0f) },
    QuadVertex2D { float2(-1.0f,  1.0f), float2(0.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f,  1.0f), float2(1.0f, 1.0f) },
    QuadVertex2D { float2( 1.0f, -1.0f), float2(1.0f, 0.0f) },
};

void deferred_pass::init()
{
    std::string source;

    deferred_ibo.create();
    deferred_ibo.storage(sizeof(quad_inds), quad_inds, 0);

    deferred_vbo.create();
    deferred_vbo.storage(sizeof(quad_verts), quad_verts, 0);

    deferred_vao.create();
    deferred_vao.setIndexBuffer(deferred_ibo);
    deferred_vao.setVertexBuffer(0, deferred_vbo, sizeof(QuadVertex2D));
    deferred_vao.enableAttribute(0, true);
    deferred_vao.setAttributeFormat(0, GL_FLOAT, 2, offsetof(QuadVertex2D, position), false);
    deferred_vao.setAttributeBinding(0, 0);
    deferred_vao.enableAttribute(1, true);
    deferred_vao.setAttributeFormat(1, GL_FLOAT, 2, offsetof(QuadVertex2D, texcoord), false);
    deferred_vao.setAttributeBinding(1, 0);

    deferred_drawcmd.create();
    deferred_drawcmd.set(GL_TRIANGLES, GL_UNSIGNED_BYTE, math::arraySize(quad_inds), 1, 0, 0, 0);

    deferred_ubuffer.create();
    deferred_ubuffer.storage(sizeof(UBufferData_DeferredPass), nullptr, GL_DYNAMIC_STORAGE_BIT);

    deferred_shaders[0].create();
    if(!fs::readText("shaders/deferred.vert.glsl", source) || !deferred_shaders[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();

    deferred_shaders[1].create();
    if(!fs::readText("shaders/deferred.frag.glsl", source) || !deferred_shaders[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();

    deferred_pipeline.create();
    deferred_pipeline.stage(deferred_shaders[0]);
    deferred_pipeline.stage(deferred_shaders[1]);

    deferred_samplers[0].create();
    deferred_samplers[0].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    deferred_samplers[0].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    deferred_samplers[0].parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    deferred_samplers[0].parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    deferred_samplers[1].create();
    deferred_samplers[1].parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    deferred_samplers[1].parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    deferred_samplers[1].parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    deferred_samplers[1].parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    deferred_samplers[1].parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
}

void deferred_pass::shutdown()
{
    deferred_samplers[1].destroy();
    deferred_samplers[0].destroy();
    deferred_pipeline.destroy();
    deferred_shaders[1].destroy();
    deferred_shaders[0].destroy();
    deferred_ubuffer.destroy();
    deferred_drawcmd.destroy();
    deferred_vao.destroy();
    deferred_vbo.destroy();
    deferred_ibo.destroy();
}

void deferred_pass::draw()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int sw, sh;
    shadow_manager::getShadowMap().getSize(sw, sh);

    UBufferData_DeferredPass deferred_ubuffer_data = {};
    deferred_ubuffer_data.tweaks = float4(sw, sh, 0.0f, 0.0f);
    deferred_ubuffer_data.light_direction = float4(shadow_manager::getLightDirection(), 0.0f);
    deferred_ubuffer_data.light_color = float4(shadow_manager::getLightColor(), 0.0f);
    deferred_ubuffer_data.ambient = float4(0.25f, 0.25f, 0.25f, 0.0f);
    deferred_ubuffer.write(0, sizeof(UBufferData_DeferredPass), &deferred_ubuffer_data);

    globals::solid_gbuffer.getAlbedo().bind(0);
    globals::solid_gbuffer.getNormal().bind(1);
    globals::solid_gbuffer.getPosition().bind(2);
    globals::solid_gbuffer.getShadowProjCoord().bind(3);
    shadow_manager::getShadowMap().getShadow().bind(4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, deferred_ubuffer.get());
    deferred_samplers[0].bind(0);
    deferred_samplers[0].bind(1);
    deferred_samplers[0].bind(2);
    deferred_samplers[0].bind(3);
    deferred_samplers[1].bind(4);
    deferred_pipeline.bind();
    deferred_vao.bind();
    deferred_drawcmd.invoke();
}
