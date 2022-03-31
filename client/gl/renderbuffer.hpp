/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/object.hpp>
#include <client/gl/pixel_format.hpp>

namespace gl
{
class Renderbuffer final : public Object<Renderbuffer> {
public:
    Renderbuffer() = default;
    Renderbuffer(Renderbuffer &&rhs);
    Renderbuffer &operator=(Renderbuffer &&rhs);
    void create();
    void destroy();
    void storage(int width, int height, PixelFormat format);
};
} // namespace gl

inline gl::Renderbuffer::Renderbuffer(gl::Renderbuffer &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Renderbuffer &gl::Renderbuffer::operator=(gl::Renderbuffer &&rhs)
{
    gl::Renderbuffer copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::Renderbuffer::create()
{
    destroy();
    glCreateRenderbuffers(1, &handle);
}

inline void gl::Renderbuffer::destroy()
{
    if(handle) {
        glDeleteRenderbuffers(1, &handle);
        handle = 0;
    }
}

inline void gl::Renderbuffer::storage(int width, int height, PixelFormat format)
{
    glNamedRenderbufferStorage(handle, gl::detail::getPixelFormatGPU(format), width, height);
}
