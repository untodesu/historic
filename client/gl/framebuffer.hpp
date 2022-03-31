/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/renderbuffer.hpp>
#include <client/gl/texture.hpp>
#include <common/math/constexpr.hpp>

namespace gl
{
class Framebuffer final : public Object<Framebuffer> {
public:
    Framebuffer() = default;
    Framebuffer(Framebuffer &&rhs);
    Framebuffer &operator=(Framebuffer &&rhs);
    void create();
    void destroy();
    void attach(GLenum attachment, const Texture2D &texture);
    void attach(GLenum attachment, const Renderbuffer &rbo);
    bool complete() const;
    void bind() const;
    template<typename... args_type>
    void setFragmentTargets(args_type &&... args);
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

inline void gl::Framebuffer::attach(GLenum attachment, const gl::Texture2D &texture)
{
    glNamedFramebufferTexture(handle, attachment, texture.get(), 0);
}

inline void gl::Framebuffer::attach(GLenum attachment, const gl::Renderbuffer &rbo)
{
    glNamedFramebufferRenderbuffer(handle, attachment, GL_RENDERBUFFER, rbo.get());
}

inline bool gl::Framebuffer::complete() const
{
    return glCheckNamedFramebufferStatus(handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

inline void gl::Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

template<typename... args_type>
inline void gl::Framebuffer::setFragmentTargets(args_type &&... args)
{
    const GLenum attachments[] = { static_cast<GLenum>(args)... };
    glNamedFramebufferDrawBuffers(handle, static_cast<GLsizei>(math::arraySize(attachments)), attachments);
}

inline void gl::Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
