/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/comp/static_chunk_mesh_component.hpp>
#include <client/chunks.hpp>
#include <client/gbuffer.hpp>
#include <client/gl/pipeline.hpp>
#include <client/gl/sampler.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/terrain_renderer.hpp>
#include <client/view.hpp>
#include <common/comp/chunk_component.hpp>
#include <common/util/include.hpp>
#include <exception>
#include <glad/gl.h>

struct GBufferPass_Uniforms final {
    matrix4f_t vpmatrix { MATRIX4F_IDENT };
    vector4f_t cpos_world { VECTOR4F_ZERO };
};

static struct {
    gl::Shader shaders[2];
    gl::Pipeline pipeline;
    gl::Sampler sampler;
    gl::Buffer uniforms;
} gbuffer_pass;

void terrain_renderer::initialize()
{
    std::string glsl_source;

    gbuffer_pass.shaders[0].create();
    gbuffer_pass.shaders[1].create();

    if(!util::include("shaders/gbuffer_terrain.vert", glsl_source) || !gbuffer_pass.shaders[0].glsl(GL_VERTEX_SHADER, glsl_source))
        std::terminate();
    if(!util::include("shaders/gbuffer_terrain.frag", glsl_source) || !gbuffer_pass.shaders[1].glsl(GL_FRAGMENT_SHADER, glsl_source))
        std::terminate();

    gbuffer_pass.pipeline.create();
    gbuffer_pass.pipeline.stage(gbuffer_pass.shaders[0]);
    gbuffer_pass.pipeline.stage(gbuffer_pass.shaders[1]);

    gbuffer_pass.sampler.create();
    gbuffer_pass.sampler.parameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    gbuffer_pass.sampler.parameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    gbuffer_pass.sampler.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gbuffer_pass.sampler.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    gbuffer_pass.uniforms.create();
    gbuffer_pass.uniforms.storage(sizeof(GBufferPass_Uniforms), nullptr, GL_DYNAMIC_STORAGE_BIT);
}

void terrain_renderer::shutdown()
{
    gbuffer_pass.uniforms.destroy();
    gbuffer_pass.sampler.destroy();
    gbuffer_pass.pipeline.destroy();
    gbuffer_pass.shaders[1].destroy();
    gbuffer_pass.shaders[0].destroy();
}

// TODO: move this to common/math
static bool isInFrustum(const math::Frustum &frustum, const vector3f_t &position, const chunk_pos_t &cpos)
{
    const vector3f_t wpos = world::getChunkWorldPosition(cpos);
    if(math::isInBB(position, wpos, wpos + vector3f_t(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    if(frustum.point(wpos + vector3f_t(0.0f, 0.0f, 0.0f)))
        return true;
    if(frustum.point(wpos + vector3f_t(0.0f, 0.0f, CHUNK_SIZE)))
        return true;
    if(frustum.point(wpos + vector3f_t(0.0f, CHUNK_SIZE, 0.0f)))
        return true;
    if(frustum.point(wpos + vector3f_t(0.0f, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    if(frustum.point(wpos + vector3f_t(CHUNK_SIZE, 0.0f, 0.0f)))
        return true;
    if(frustum.point(wpos + vector3f_t(CHUNK_SIZE, 0.0f, CHUNK_SIZE)))
        return true;
    if(frustum.point(wpos + vector3f_t(CHUNK_SIZE, CHUNK_SIZE, 0.0f)))
        return true;
    if(frustum.point(wpos + vector3f_t(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)))
        return true;
    return false;
}

void terrain_renderer::renderWorld()
{
    const auto group = globals::registry.group(entt::get<ChunkComponent, StaticChunkMeshComponent>);
    if(group.empty())
        return;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    GBufferPass_Uniforms uniforms = {};
    uniforms.vpmatrix = view::viewProjectionMatrix();

    gbuffer_pass.pipeline.bind();
    gbuffer_pass.sampler.bind(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gbuffer_pass.uniforms.get());
    globals::main_gbuffer.getFramebuffer().bind();
    globals::terrain_atlas.get().bind(0);

    const vector2i_t size = screen::size2i();
    glViewport(0, 0, size.x, size.y);

    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const math::Frustum &frustum = view::frustum();
    const vector3f_t &position = view::position();

    for(const auto [entity, chunk, mesh] : group.each()) {
        if(isInFrustum(frustum, position, chunk.cpos)) {
            uniforms.cpos_world = vector4f_t(world::getChunkWorldPosition(chunk.cpos), 0.0f);
            gbuffer_pass.uniforms.write(0, sizeof(GBufferPass_Uniforms), &uniforms);

            // FIXME: an inline function maybe?
            // FIXME: we should really support alpha testing here.
            if(mesh.cube) {
                mesh.cube->vao.bind();
                mesh.cube->cmd.invoke();
                globals::vertices_drawn += mesh.cube->cmd.size();
            }
        }
    }
}
