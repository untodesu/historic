/*
 * gbuffer.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <client/render/gl/framebuffer.hpp>

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
