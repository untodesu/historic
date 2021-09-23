/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/texture.hpp>

namespace gl
{
class Framebuffer final : public Object<Framebuffer> {
public:
    Framebuffer() = default;
    Framebuffer(Framebuffer &&rhs);
    Framebuffer &operator=(Framebuffer &&rhs);
    void create();
    void destroy();
    void color(uint32_t id, const Texture2D &texture);
    void depth(const Texture2D &texture);
    void stencil(const Texture2D &texture);
    bool complete() const;
    void bind() const;
    static void unbind();
};
} // namespace gl

inline gl::Framebuffer::Framebuffer(gl::Framebuffer &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Framebuffer &gl::Framebuffer::operator=(gl::Framebuffer &&rhs)
{
    gl::Framebuffer copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::Framebuffer::create()
{
    destroy();
    glCreateFramebuffers(1, &handle);
}

inline void gl::Framebuffer::destroy()
{
    if(handle) {
        glDeleteFramebuffers(1, &handle);
        handle = 0;
    }
}

inline void gl::Framebuffer::color(uint32_t id, const gl::Texture2D &texture)
{
    glNamedFramebufferTexture(handle, GL_COLOR_ATTACHMENT0 + id, texture.get(), 0);
}

inline void gl::Framebuffer::depth(const gl::Texture2D &texture)
{
    glNamedFramebufferTexture(handle, GL_DEPTH_ATTACHMENT, texture.get(), 0);
}

inline void gl::Framebuffer::stencil(const gl::Texture2D &texture)
{
    glNamedFramebufferTexture(handle, GL_STENCIL_ATTACHMENT, texture.get(), 0);
}

inline bool gl::Framebuffer::complete() const
{
    return glCheckNamedFramebufferStatus(handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

inline void gl::Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

inline void gl::Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
