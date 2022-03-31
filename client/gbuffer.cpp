/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/gbuffer.hpp>

void GBuffer::create(int width, int height)
{
    destroy();

    // ID: 0.
    // Format: RGBA 32-bit float.
    // Comment: color data.
    albedo_attachment.create();
    albedo_attachment.storage(width, height, gl::PixelFormat::R32G32B32A32_FLOAT);

    // ID: 1.
    // Format: XYZ 32-bit float.
    // Comment: vertex normals.
    normal_attachment.create();
    normal_attachment.storage(width, height, gl::PixelFormat::R32G32B32_FLOAT);

    // ID: none.
    // Format: 24-bit depth.
    // Comment: depth buffer stub.
    renderbuffer.create();
    renderbuffer.storage(width, height, gl::PixelFormat::D24_UNORM);

    framebuffer.create();

    // Attach textures
    framebuffer.attach(GL_COLOR_ATTACHMENT0, albedo_attachment);
    framebuffer.attach(GL_COLOR_ATTACHMENT1, normal_attachment);
    framebuffer.setFragmentTargets(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

    // Attach depth
    framebuffer.attach(GL_DEPTH_ATTACHMENT, renderbuffer);
}

void GBuffer::destroy()
{
    framebuffer.destroy();
    renderbuffer.destroy();
    normal_attachment.destroy();
    albedo_attachment.destroy();
}

const gl::Texture2D &GBuffer::getAlbedoAttachment() const
{
    return albedo_attachment;
}

const gl::Texture2D &GBuffer::getNormalAttachment() const
{
    return normal_attachment;
}

const gl::Renderbuffer &GBuffer::getRenderbuffer() const
{
    return renderbuffer;
}

const gl::Framebuffer &GBuffer::getFramebuffer() const
{
    return framebuffer;
}
