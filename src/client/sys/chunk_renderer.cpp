/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <filesystem.hpp>
#include <client/comp/chunk_mesh.hpp>
#include <client/sys/proj_view.hpp>
#include <client/sys/chunk_renderer.hpp>
#include <client/globals.hpp>
#include <spdlog/spdlog.h>
#include <client/vertex.hpp>
#include <math/util.hpp>
#include <client/gl/pipeline.hpp>
#include <client/gl/sampler.hpp>
#include <client/gl/shader.hpp>
#include <client/atlas.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/world.hpp>
#include <client/screen.hpp>
#include <client/gbuffer.hpp>
#include <client/shadowmap.hpp>

struct alignas(16) UBOData_Shadow final {
    float4x4 projview;
    float3 chunkpos;
};

struct alignas(16) UBOData_GBuffer final {
    float4x4 projview;
    float4x4 projview_shadow;
    float3 chunkpos;
};

static gl::Buffer shadow_ubo;
static gl::Shader shadow_shaders[2];
static gl::Pipeline shadow_pipeline;
static gl::Buffer gbuffer_ubo;
static gl::Shader gbuffer_shaders[2];
static gl::Pipeline gbuffer_pipeline;
static gl::Sampler gbuffer_samplers[2];

void chunk_renderer::init()
{
    std::string source;

    shadow_ubo.create();
    shadow_ubo.storage(sizeof(UBOData_Shadow), nullptr, GL_DYNAMIC_STORAGE_BIT);

    shadow_shaders[0].create();
    if(!fs::readText("shaders/chunk_shadow.vert.glsl", source) || !shadow_shaders[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();

    shadow_shaders[1].create();
    if(!fs::readText("shaders/chunk_shadow.frag.glsl", source) || !shadow_shaders[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();

    shadow_pipeline.create();
    shadow_pipeline.stage(shadow_shaders[0]);
    shadow_pipeline.stage(shadow_shaders[1]);

    gbuffer_ubo.create();
    gbuffer_ubo.storage(sizeof(UBOData_GBuffer), nullptr, GL_DYNAMIC_STORAGE_BIT);

    gbuffer_shaders[0].create();
    if(!fs::readText("shaders/chunk_gbuffer.vert.glsl", source) || !gbuffer_shaders[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();

    gbuffer_shaders[1].create();
    if(!fs::readText("shaders/chunk_gbuffer.frag.glsl", source) || !gbuffer_shaders[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();

    gbuffer_pipeline.create();
    gbuffer_pipeline.stage(gbuffer_shaders[0]);
    gbuffer_pipeline.stage(gbuffer_shaders[1]);

    gbuffer_samplers[0].create();
    gbuffer_samplers[0].parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gbuffer_samplers[0].parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    gbuffer_samplers[1].create();
    gbuffer_samplers[1].parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gbuffer_samplers[1].parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void chunk_renderer::shutdown()
{
    gbuffer_samplers[1].destroy();
    gbuffer_samplers[0].destroy();
    gbuffer_pipeline.destroy();
    gbuffer_shaders[1].destroy();
    gbuffer_shaders[0].destroy();
    gbuffer_ubo.destroy();

    shadow_pipeline.destroy();
    shadow_shaders[1].destroy();
    shadow_shaders[0].destroy();
    shadow_ubo.destroy();
}

// UNDONE: move this somewhere else
static inline bool isInFrustum(const Frustum &frustum, const float3 &position, const chunkpos_t &cp)
{
    const float3 wp = toWorldPos(cp);
    if(math::isInBB(position, wp, wp + float3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    if(frustum.point(wp + float3(0.0f, 0.0f, 0.0f)))
        return true;
    if(frustum.point(wp + float3(0.0f, 0.0f, CHUNK_SIZE)))
        return true;
    if(frustum.point(wp + float3(0.0f, CHUNK_SIZE, 0.0f)))
        return true;
    if(frustum.point(wp + float3(0.0f, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    if(frustum.point(wp + float3(CHUNK_SIZE, 0.0f, 0.0f)))
        return true;
    if(frustum.point(wp + float3(CHUNK_SIZE, 0.0f, CHUNK_SIZE)))
        return true;
    if(frustum.point(wp + float3(CHUNK_SIZE, CHUNK_SIZE, 0.0f)))
        return true;
    if(frustum.point(wp + float3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    return false;
}

void chunk_renderer::draw()
{
    int width, height;
    const auto group = cl_globals::registry.group(entt::get<ChunkMeshComponent, ChunkComponent>);
    if(group.empty())
        return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //
    // SHADOW PASS
    //
    UBOData_Shadow shadow_ubo_data = {};
    shadow_ubo_data.projview = proj_view::matrixShadow();
    shadow_ubo.write(0, sizeof(UBOData_Shadow), &shadow_ubo_data);

    const Frustum &shadow_frustum = proj_view::frustumShadow();

    shadow_pipeline.bind();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, shadow_ubo.get());
    cl_globals::shadowmap_0.getFBO().bind();

    glDisable(GL_CULL_FACE);

    cl_globals::shadowmap_0.getSize(width, height);
    glViewport(0, 0, width, height);

    glClear(GL_DEPTH_BUFFER_BIT);

    for(const auto [entity, mesh, chunk] : group.each()) {
        if(isInFrustum(shadow_frustum, FLOAT3_ZERO, chunk.position)) {
            shadow_ubo_data.chunkpos = toWorldPos(chunk.position);
            shadow_ubo.write(offsetof(UBOData_Shadow, chunkpos), sizeof(float3), &shadow_ubo_data.chunkpos);
            mesh.vao.bind();
            mesh.cmd.invoke();
        }
    }

    //
    // GBUFFER PASS
    //
    UBOData_GBuffer gbuffer_ubo_data = {};
    gbuffer_ubo_data.projview = proj_view::matrix();
    gbuffer_ubo_data.projview_shadow = proj_view::matrixShadow();
    gbuffer_ubo.write(0, sizeof(UBOData_GBuffer), &gbuffer_ubo_data);

    const float3 &gbuffer_position = proj_view::position();
    const Frustum &gbuffer_frustum = proj_view::frustum();

    gbuffer_pipeline.bind();
    gbuffer_samplers[0].bind(0);
    gbuffer_samplers[1].bind(1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gbuffer_ubo.get());
    cl_globals::chunk_gbuffer_0.getFBO().bind();
    cl_globals::solid_textures.getTexture().bind(0);
    cl_globals::shadowmap_0.getShadow().bind(1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(const auto [entity, mesh, chunk] : group.each()) {
        if(isInFrustum(gbuffer_frustum, gbuffer_position, chunk.position)) {
            gbuffer_ubo_data.chunkpos = toWorldPos(chunk.position);
            gbuffer_ubo.write(offsetof(UBOData_GBuffer, chunkpos), sizeof(float3), &gbuffer_ubo_data.chunkpos);
            mesh.vao.bind();
            mesh.cmd.invoke();
        }
    }
}
