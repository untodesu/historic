/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/client/gl/framebuffer.hpp>

class ShadowMap final {
public:
    void init(int width, int height, gl::PixelFormat format);
    void shutdown();

    int getWidth() const;
    int getHeight() const;
    void getSize(int &width, int &height) const;
    const gl::Framebuffer &getFBO() const;
    const gl::Texture2D &getShadow() const;

private:
    int width, height;
    gl::Framebuffer fbo;
    gl::Texture2D shadow;
};

inline int ShadowMap::getWidth() const
{
    return width;
}

inline int ShadowMap::getHeight() const
{
    return height;
}

inline void ShadowMap::getSize(int &width, int &height) const
{
    width = this->width;
    height = this->height;
}

inline const gl::Framebuffer &ShadowMap::getFBO() const
{
    return fbo;
}

inline const gl::Texture2D &ShadowMap::getShadow() const
{
    return shadow;
}
