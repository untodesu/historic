/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/client/gl/object.hpp>
#include <game/client/gl/pixelformat.hpp>

namespace gl
{
class RenderBuffer final : public Object<RenderBuffer> {
public:
    RenderBuffer() = default;
    RenderBuffer(RenderBuffer &&rhs);
    RenderBuffer &operator=(RenderBuffer &&rhs);
    void create();
    void destroy();
    void storage(int width, int height, PixelFormat format);
};
} // namespace gl

inline gl::RenderBuffer::RenderBuffer(gl::RenderBuffer &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::RenderBuffer &gl::RenderBuffer::operator=(gl::RenderBuffer &&rhs)
{
    gl::RenderBuffer copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::RenderBuffer::create()
{
    destroy();
    glCreateRenderbuffers(1, &handle);
}

inline void gl::RenderBuffer::destroy()
{
    if(handle) {
        glDeleteRenderbuffers(1, &handle);
        handle = 0;
    }
}

inline void gl::RenderBuffer::storage(int width, int height, PixelFormat format)
{
    glNamedRenderbufferStorage(handle, gl::getInternalFormat(format), width, height);
}
