/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/framebuffer.hpp>

class GBuffer final {
public:
    void create(int width, int height);
    void destroy();

    const gl::Texture2D &getAlbedoAttachment() const;
    const gl::Texture2D &getNormalAttachment() const;

    const gl::Renderbuffer &getRenderbuffer() const;
    const gl::Framebuffer &getFramebuffer() const;

private:
    gl::Texture2D albedo_attachment;
    gl::Texture2D normal_attachment;
    gl::Renderbuffer renderbuffer;
    gl::Framebuffer framebuffer;
};
