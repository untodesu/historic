/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <memory>
#include <uvre/uvre.hpp>

class UVREDeviceWrapper final {
public:
    void set(uvre::IRenderDevice *device);
    uvre::IRenderDevice *get();
    uvre::IRenderDevice *operator->();

    std::shared_ptr<uvre::Shader> createSharedShader(const uvre::ShaderInfo &info);
    std::shared_ptr<uvre::Pipeline> createSharedPipeline(const uvre::PipelineInfo &info);
    std::shared_ptr<uvre::Buffer> createSharedBuffer(const uvre::BufferInfo &info);
    std::shared_ptr<uvre::Sampler> createSharedSampler(const uvre::SamplerInfo &info);
    std::shared_ptr<uvre::Texture> createSharedTexture(const uvre::TextureInfo &info);
    std::shared_ptr<uvre::RenderTarget> createSharedRenderTarget(const uvre::RenderTargetInfo &info);

private:
    uvre::IRenderDevice *device { nullptr };
};
