/*
 * player_renderer.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/components/local_player.hpp>
#include <client/render/gl/buffer.hpp>
#include <client/render/gl/pipeline.hpp>
#include <client/render/gl/sampler.hpp>
#include <client/render/gl/shader.hpp>
#include <client/render/gbuffer.hpp>
#include <client/render/shadowmap.hpp>
#include <client/systems/player_renderer.hpp>
#include <client/systems/proj_view.hpp>
#include <client/globals.hpp>

struct UBOData_Shadow final {
    float4x4 projview;
    float4 chunkpos;
};

struct UBOData_GBuffer final {
    float4x4 projview;
    float4x4 projview_shadow;
    float4 chunkpos;
};

static gl::Buffer shadow_ubo;
static gl::Shader shadow_shaders[2];
static gl::Pipeline shadow_pipeline;
static gl::Buffer gbuffer_ubo;
static gl::Shader gbuffer_shaders[2];
static gl::Pipeline gbuffer_pipeline;
static gl::Sampler gbuffer_samplers[2];
