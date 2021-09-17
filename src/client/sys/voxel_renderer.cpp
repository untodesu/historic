/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/sys/proj_view.hpp>
#include <client/sys/voxel_renderer.hpp>
#include <client/shaderlib.hpp>
#include <client/globals.hpp>
#include <client/world.hpp>
#include <shared/comp/chunk.hpp>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>
#include <math/vertex.hpp>

constexpr static const char *VERT_NAME = "shaders/voxel.vert.glsl";
constexpr static const char *FRAG_NAME = "shaders/voxel.frag.glsl";

struct alignas(16) UBufferData final {
    float4x4_t projview;
    chunkpos_t chunkpos;
};

static uvre::ICommandList *commands = nullptr;
static uvre::Shader shaders[2] = { nullptr, nullptr };
static uvre::Pipeline pipeline = nullptr;
static uvre::Sampler sampler = nullptr;
static uvre::Buffer ubuffer = nullptr;

void voxel_renderer::init()
{
    commands = globals::render_device->createCommandList();
    if(!commands) 
        std::terminate();

    shaders[0] = shaderlib::load(VERT_NAME, uvre::ShaderFormat::SOURCE_GLSL, uvre::ShaderStage::VERTEX);
    shaders[1] = shaderlib::load(FRAG_NAME, uvre::ShaderFormat::SOURCE_GLSL, uvre::ShaderStage::FRAGMENT);
    if(!shaders[0] || !shaders[1])
        std::terminate();
    
    uvre::VertexAttrib attributes[2] = {};
    attributes[0] = uvre::VertexAttrib { 0, uvre::VertexAttribType::FLOAT32, 3, offsetof(Vertex, position), false };
    attributes[1] = uvre::VertexAttrib { 1, uvre::VertexAttribType::FLOAT32, 2, offsetof(Vertex, texcoord), false };

    uvre::PipelineInfo pipeline_info = {};
    pipeline_info.blending.enabled = false;
    pipeline_info.depth_testing.enabled = true;
    pipeline_info.depth_testing.func = uvre::DepthFunc::LESS_OR_EQUAL;
    pipeline_info.face_culling.enabled = true;
    pipeline_info.face_culling.flags = uvre::CULL_BACK;
    pipeline_info.index_type = uvre::IndexType::INDEX32;
    pipeline_info.primitive_mode = uvre::PrimitiveMode::TRIANGLES;
    pipeline_info.fill_mode = uvre::FillMode::FILLED;
    pipeline_info.vertex_stride = sizeof(Vertex);
    pipeline_info.num_vertex_attribs = 2;
    pipeline_info.vertex_attribs = attributes;
    pipeline_info.num_shaders = 2;
    pipeline_info.shaders = shaders;

    pipeline = globals::render_device->createPipeline(pipeline_info);
    if(!pipeline)
        std::terminate();

    uvre::SamplerInfo sampler_info = {};
    sampler_info.flags = uvre::SAMPLER_CLAMP_S | uvre::SAMPLER_CLAMP_T;

    sampler = globals::render_device->createSampler(sampler_info);
    if(!sampler)
        std::terminate();

    uvre::BufferInfo ubuffer_info = {};
    ubuffer_info.type = uvre::BufferType::DATA_BUFFER;
    ubuffer_info.size = sizeof(UBufferData);

    ubuffer = globals::render_device->createBuffer(ubuffer_info);
    if(!ubuffer)
        std::terminate();
}

void voxel_renderer::shutdown()
{
    globals::render_device->destroyCommandList(commands);
    ubuffer = nullptr;
    sampler = nullptr;
    pipeline = nullptr;
    shaders[1] = nullptr;
    shaders[0] = nullptr;
    commands = nullptr;
}

void voxel_renderer::update()
{
    entt::registry &registry = client_world::registry();

    globals::render_device->startRecording(commands);

    UBufferData ubuffer_data = {};
    ubuffer_data.projview = proj_view::get();

    commands->writeBuffer(ubuffer, offsetof(UBufferData, projview), sizeof(ubuffer_data.projview), &ubuffer_data.projview);
    commands->bindPipeline(pipeline);
    commands->bindUniformBuffer(ubuffer, 0);
    commands->bindSampler(sampler, 0);

    auto group = registry.group(entt::get<VoxelMeshComponent, ChunkComponent>);
    for(const auto [entity, mesh, chunk] : group.each()) {
        ubuffer_data.chunkpos = chunk.position;
        commands->writeBuffer(ubuffer, offsetof(UBufferData, chunkpos), sizeof(ubuffer_data.chunkpos), &ubuffer_data.chunkpos);
        for(const VoxelMesh &part : mesh.data) {
            commands->bindIndexBuffer(part.ibo);
            commands->bindVertexBuffer(part.vbo);
            commands->bindTexture(part.texture, 0);
            commands->idraw(part.count, 1, 0, 0, 0);
        }
    }

    globals::render_device->submit(commands);
}
