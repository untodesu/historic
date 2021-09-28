/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/client/gl/framebuffer.hpp>

class GBuffer final {
public:
    void init(int width, int height);
    void shutdown();

    const gl::Framebuffer &getFBO() const;
    const gl::Texture2D &getAlbedo() const;
    const gl::Texture2D &getNormal() const;
    const gl::Texture2D &getPosition() const;
    const gl::Texture2D &getShadowProjCoord() const;

private:
    gl::Framebuffer fbo;
    gl::RenderBuffer rbo;
    gl::Texture2D albedo;
    gl::Texture2D normal;
    gl::Texture2D position;
    gl::Texture2D shadow_projcoord;
};

inline const gl::Framebuffer &GBuffer::getFBO() const
{
    return fbo;
}

inline const gl::Texture2D &GBuffer::getAlbedo() const
{
    return albedo;
}

inline const gl::Texture2D &GBuffer::getNormal() const
{
    return normal;
}

inline const gl::Texture2D &GBuffer::getPosition() const
{
    return position;
}

inline const gl::Texture2D &GBuffer::getShadowProjCoord() const
{
    return shadow_projcoord;
}
