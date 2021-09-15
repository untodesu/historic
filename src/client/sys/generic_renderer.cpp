/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/generic_mesh.hpp>
#include <client/sys/generic_renderer.hpp>
#include <client/sys/proj_view.hpp>
#include <client/client_globals.hpp>
#include <client/vertex.hpp>
#include <fs.hpp>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>

struct alignas(16) UBufferData {
    float4x4_t model;
    float4x4_t projview;
};

static uvre::ICommandList *commands = nullptr;
static uvre::Pipeline *pipeline = nullptr;
static uvre::Shader *shaders[2] = { nullptr, nullptr };
static uvre::Buffer *ubuffer = nullptr;
static uvre::Sampler *sampler = nullptr;

void generic_renderer::init()
{
    commands = globals::render_device->createCommandList();
    if(!commands) {
        spdlog::error("Unable to create a command list");
        std::terminate();
    }

    std::string vert_src;
    if(!fs::readText("shaders/generic_mesh.vert.glsl", vert_src)) {
        spdlog::error("Unable to read generic_mesh.vert.glsl");
        std::terminate();
    }

    std::string frag_src;
    if(!fs::readText("shaders/generic_mesh.frag.glsl", frag_src)) {
        spdlog::error("Unable to read generic_mesh.frag.glsl");
        std::terminate();
    }

    uvre::ShaderInfo vert_info = {};
    vert_info.stage = uvre::ShaderStage::VERTEX;
    vert_info.format = uvre::ShaderFormat::SOURCE_GLSL;
    vert_info.code = vert_src.c_str();

    shaders[0] = globals::render_device->createShader(vert_info);
    if(!shaders[0]) {
        spdlog::info("Unable to compile generic_mesh.vert.glsl");
        std::terminate();
    }

    uvre::ShaderInfo frag_info = {};
    frag_info.stage = uvre::ShaderStage::FRAGMENT;
    frag_info.format = uvre::ShaderFormat::SOURCE_GLSL;
    frag_info.code = frag_src.c_str();

    shaders[1] = globals::render_device->createShader(frag_info);
    if(!shaders[1]) {
        spdlog::info("Unable to compile generic_mesh.frag.glsl");
        std::terminate();
    }

    uvre::VertexAttrib attributes[2] = {};
    attributes[0] = uvre::VertexAttrib { 0, uvre::VertexAttribType::FLOAT32, 3, offsetof(Vertex, position), false };
    attributes[1] = uvre::VertexAttrib { 1, uvre::VertexAttribType::FLOAT32, 2, offsetof(Vertex, texcoord), false };

    uvre::PipelineInfo pipeline_info = {};
    pipeline_info.blending.enabled = false;
    pipeline_info.depth_testing.enabled = false;
    pipeline_info.face_culling.enabled = false;
    pipeline_info.index_type = uvre::IndexType::INDEX16;
    pipeline_info.primitive_mode = uvre::PrimitiveMode::TRIANGLES;
    pipeline_info.fill_mode = uvre::FillMode::FILLED;
    pipeline_info.vertex_stride = sizeof(Vertex);
    pipeline_info.num_vertex_attribs = 2;
    pipeline_info.vertex_attribs = attributes;
    pipeline_info.num_shaders = 2;
    pipeline_info.shaders = shaders;

    pipeline = globals::render_device->createPipeline(pipeline_info);
    if(!pipeline) {
        spdlog::error("Unable to create a pipeline");
        std::terminate();
    }

    uvre::BufferInfo ubuffer_info = {};
    ubuffer_info.type = uvre::BufferType::DATA_BUFFER;
    ubuffer_info.size = sizeof(UBufferData);

    ubuffer = globals::render_device->createBuffer(ubuffer_info);
    if(!ubuffer) {
        spdlog::info("Unable to create a buffer object (UBuffer)");
        std::terminate();
    }

    uvre::SamplerInfo sampler_info = {};
    sampler_info.flags = uvre::SAMPLER_CLAMP_S | uvre::SAMPLER_CLAMP_T | uvre::SAMPLER_FILTER;

    sampler = globals::render_device->createSampler(sampler_info);
    if(!sampler) {
        spdlog::info("Unable to create a sampler object");
        std::terminate();
    }
}

void generic_renderer::shutdown()
{
    globals::render_device->destroySampler(sampler);
    globals::render_device->destroyBuffer(ubuffer);
    globals::render_device->destroyPipeline(pipeline);
    globals::render_device->destroyShader(shaders[1]);
    globals::render_device->destroyShader(shaders[0]);
    globals::render_device->destroyCommandList(commands);

    shaders[0] = nullptr;
    shaders[1] = nullptr;
    pipeline = nullptr;
    ubuffer = nullptr;
    sampler = nullptr;
}

void generic_renderer::update(entt::registry &registry)
{
    globals::render_device->startRecording(commands);

    UBufferData ubuffer_data = {};
    ubuffer_data.projview = proj_view::get();
    if(!commands->writeBuffer(ubuffer, offsetof(UBufferData, projview), sizeof(UBufferData::projview), &ubuffer_data.projview))
        std::terminate();

    commands->bindPipeline(pipeline);
    commands->bindUniformBuffer(ubuffer, 0);

    const auto view = registry.view<GenericMeshComponent>();
    for(const auto [entity, mesh] : view.each()) {
        ubuffer_data.model = FLOAT4X4_IDENTITY;
        commands->writeBuffer(ubuffer, offsetof(UBufferData, model), sizeof(UBufferData::model), &ubuffer_data.model);
        commands->bindVertexBuffer(mesh.vbo);
        
        if(mesh.ibo) {
            commands->bindSampler(sampler, 0);
            commands->bindTexture(mesh.tex.get(), 0);
            commands->bindIndexBuffer(mesh.ibo);
            commands->idraw(mesh.nv, 1, 0, 0, 0);
            continue;
        }

        commands->draw(mesh.nv, 1, 0, 0);
    }

    globals::render_device->submit(commands);
}
