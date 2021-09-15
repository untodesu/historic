/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/uvre_devicewrap.hpp>
#include <functional>

void UVREDeviceWrapper::set(uvre::IRenderDevice *device)
{
    this->device = device;
}

uvre::IRenderDevice *UVREDeviceWrapper::get()
{
    return device;
}

uvre::IRenderDevice *UVREDeviceWrapper::operator->()
{
    return device;
}

std::shared_ptr<uvre::Shader> UVREDeviceWrapper::createSharedShader(const uvre::ShaderInfo &info)
{
    return std::shared_ptr<uvre::Shader>(device->createShader(info), std::bind(&uvre::IRenderDevice::destroyShader, device, std::placeholders::_1));
}

std::shared_ptr<uvre::Pipeline> UVREDeviceWrapper::createSharedPipeline(const uvre::PipelineInfo &info)
{
    return std::shared_ptr<uvre::Pipeline>(device->createPipeline(info), std::bind(&uvre::IRenderDevice::destroyPipeline, device, std::placeholders::_1));
}

std::shared_ptr<uvre::Buffer> UVREDeviceWrapper::createSharedBuffer(const uvre::BufferInfo &info)
{
    return std::shared_ptr<uvre::Buffer>(device->createBuffer(info), std::bind(&uvre::IRenderDevice::destroyBuffer, device, std::placeholders::_1));
}

std::shared_ptr<uvre::Sampler> UVREDeviceWrapper::createSharedSampler(const uvre::SamplerInfo &info)
{
    return std::shared_ptr<uvre::Sampler>(device->createSampler(info), std::bind(&uvre::IRenderDevice::destroySampler, device, std::placeholders::_1));
}

std::shared_ptr<uvre::Texture> UVREDeviceWrapper::createSharedTexture(const uvre::TextureInfo &info)
{
    return std::shared_ptr<uvre::Texture>(device->createTexture(info), std::bind(&uvre::IRenderDevice::destroyTexture, device, std::placeholders::_1));
}

std::shared_ptr<uvre::RenderTarget> UVREDeviceWrapper::createSharedRenderTarget(const uvre::RenderTargetInfo &info)
{
    return std::shared_ptr<uvre::RenderTarget>(device->createRenderTarget(info), std::bind(&uvre::IRenderDevice::destroyRenderTarget, device, std::placeholders::_1));
}
