/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <exception>
#include <client/render/shadowmap.hpp>

void ShadowMap::init(int width, int height, gl::PixelFormat format)
{
    shutdown();
    
    this->width = width;
    this->height = height;
    
    shadow.create();
    shadow.storage(width, height, format);

    fbo.create();
    fbo.attach(GL_DEPTH_ATTACHMENT, shadow);
}

void ShadowMap::shutdown()
{
    fbo.destroy();
    shadow.destroy();
    width = 0;
    height = 0;
}

