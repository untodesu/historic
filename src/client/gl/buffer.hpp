/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/object.hpp>

namespace gl
{
class Buffer final : public Object<Buffer> {
public:
    Buffer() = default;
    Buffer(Buffer &&rhs);
    Buffer &operator=(Buffer &&rhs);
    void create();
    void destroy();
    void storage(size_t size, const void *data, uint32_t flags);
    void resize(size_t new_size, const void *data, uint32_t usage);
    void write(size_t offset, size_t size, const void *data);
};
} // namespace gl

inline gl::Buffer::Buffer(gl::Buffer &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Buffer &gl::Buffer::operator=(gl::Buffer &&rhs)
{
    gl::Buffer copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::Buffer::create()
{
    destroy();
    glCreateBuffers(1, &handle);
}

inline void gl::Buffer::destroy()
{
    if(handle) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }
}

inline void gl::Buffer::storage(size_t size, const void *data, uint32_t flags)
{
    glNamedBufferStorage(handle, static_cast<GLsizeiptr>(size), data, flags);
}

inline void gl::Buffer::resize(size_t new_size, const void *data, uint32_t usage)
{
    glNamedBufferData(handle, static_cast<GLsizeiptr>(new_size), data, usage);
}

inline void gl::Buffer::write(size_t offset, size_t size, const void *data)
{
    glNamedBufferSubData(handle, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}
