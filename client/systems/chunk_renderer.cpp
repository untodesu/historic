/*
 * chunk_renderer.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/components/chunk_mesh.hpp>
#include <client/render/atlas.hpp>
#include <client/render/gbuffer.hpp>
#include <client/systems/chunk_renderer.hpp>
#include <client/systems/proj_view.hpp>
#include <client/util/include.hpp>
#include <client/globals.hpp>
#include <spdlog/spdlog.h>
#include <client/vertex.hpp>
#include <common/math/math.hpp>
#include <client/render/gl/pipeline.hpp>
#include <client/render/gl/sampler.hpp>
#include <client/render/gl/shader.hpp>
#include <shared/components/chunk.hpp>
#include <shared/world.hpp>
#include <client/screen.hpp>
#include <client/shadow_manager.hpp>
#include <client/config.hpp>

struct alignas(16) Shadow_UBO0 final {
    float4x4 proj_view;
    float4 chunk_pos;
};

struct alignas(16) GBuffer_UBO0 final {
    float4x4 proj_view;
    float4x4 proj_view_shadow;
    float4 chunk_pos;
};

static struct {
    gl::Shader stages[2];
    gl::Pipeline pipeline;
    gl::Sampler sampler;
    gl::Buffer ubo_0;
} gbuffer_ctx;

static struct {
    gl::Shader vertex_stage;
    gl::Pipeline pipeline;
    gl::Buffer ubo_0;
} shadow_ctx;

void chunk_renderer::init()
{
    std::string source;


    gbuffer_ctx.stages[0].create();
    if(!util::include("shaders/chunk_gbuffer.vert.glsl", source) || !gbuffer_ctx.stages[0].glsl(GL_VERTEX_SHADER, source))
        std::terminate();
    
    gbuffer_ctx.stages[1].create();
    if(!util::include("shaders/chunk_gbuffer.frag.glsl", source) || !gbuffer_ctx.stages[1].glsl(GL_FRAGMENT_SHADER, source))
        std::terminate();

    gbuffer_ctx.pipeline.create();
    gbuffer_ctx.pipeline.stage(gbuffer_ctx.stages[0]);
    gbuffer_ctx.pipeline.stage(gbuffer_ctx.stages[1]);

    gbuffer_ctx.sampler.create();
    gbuffer_ctx.sampler.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gbuffer_ctx.sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    gbuffer_ctx.ubo_0.create();
    gbuffer_ctx.ubo_0.storage(sizeof(GBuffer_UBO0), nullptr, GL_DYNAMIC_STORAGE_BIT);


    shadow_ctx.vertex_stage.create();
    if(!util::include("shaders/chunk_shadow.vert.glsl", source) || !shadow_ctx.vertex_stage.glsl(GL_VERTEX_SHADER, source))
        std::terminate();
    
    shadow_ctx.pipeline.create();
    shadow_ctx.pipeline.stage(shadow_ctx.vertex_stage);

    shadow_ctx.ubo_0.create();
    shadow_ctx.ubo_0.storage(sizeof(Shadow_UBO0), nullptr, GL_DYNAMIC_STORAGE_BIT);
}

void chunk_renderer::shutdown()
{
    shadow_ctx.ubo_0.destroy();
    shadow_ctx.pipeline.destroy();
    shadow_ctx.vertex_stage.destroy();

    gbuffer_ctx.ubo_0.destroy();
    gbuffer_ctx.sampler.destroy();
    gbuffer_ctx.pipeline.destroy();
    gbuffer_ctx.stages[1].destroy();
    gbuffer_ctx.stages[0].destroy();
}

// This really shouldn't be here.
// TODO: move this to common/math of whatever.
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

    const auto group = globals::registry.group(entt::get<ChunkMeshComponent, ChunkComponent>);
    if(group.empty())
        return;


    glDisable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    GBuffer_UBO0 gbuffer_ubo_0 = {};
    gbuffer_ubo_0.proj_view = proj_view::matrix();
    gbuffer_ubo_0.proj_view_shadow = proj_view::matrixShadow();
    gbuffer_ctx.ubo_0.write(0, sizeof(GBuffer_UBO0), &gbuffer_ubo_0);

    const float3 &gbuffer_position = proj_view::position();
    const Frustum &gbuffer_frustum = proj_view::frustum();

    gbuffer_ctx.pipeline.bind();
    gbuffer_ctx.sampler.bind(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gbuffer_ctx.ubo_0.get());
    globals::solid_gbuffer.getFBO().bind();
    globals::solid_textures.getTexture().bind(0);

    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(const auto [entity, mesh, chunk] : group.each()) {
        if(isInFrustum(gbuffer_frustum, gbuffer_position, chunk.position)) {
            gbuffer_ubo_0.chunk_pos = float4(toWorldPos(chunk.position), 0.0f);
            gbuffer_ctx.ubo_0.write(offsetof(GBuffer_UBO0, chunk_pos), sizeof(float4), &gbuffer_ubo_0.chunk_pos);
            mesh.vao.bind();
            mesh.cmd.invoke();
            globals::vertices_drawn += mesh.cmd.size();
        }
    }

    if(globals::config.render.draw_shadows) {
        Shadow_UBO0 shadow_ubo_0 = {};
        shadow_ubo_0.proj_view = proj_view::matrixShadow();
        shadow_ctx.ubo_0.write(0, sizeof(Shadow_UBO0), &shadow_ubo_0);

        const Frustum &shadow_frustum = proj_view::frustumShadow();

        shadow_ctx.pipeline.bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, shadow_ctx.ubo_0.get());
        shadow_manager::getShadowMap().getFBO().bind();

        shadow_manager::getShadowMap().getSize(width, height);
        glViewport(0, 0, width, height);

        const float2 &off = shadow_manager::getPolygonOffset();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(off.x, off.y);

        glDisable(GL_CULL_FACE);

        for(const auto [entity, mesh, chunk] : group.each()) {
            if(isInFrustum(shadow_frustum, FLOAT3_ZERO, chunk.position)) {
                shadow_ubo_0.chunk_pos = float4(toWorldPos(chunk.position), 0.0f);
                shadow_ctx.ubo_0.write(offsetof(Shadow_UBO0, chunk_pos), sizeof(float4), &shadow_ubo_0.chunk_pos);
                mesh.vao.bind();
                mesh.cmd.invoke();
                globals::vertices_drawn += mesh.cmd.size();
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}
