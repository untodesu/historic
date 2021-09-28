/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/client/gl/buffer.hpp>
#include <spdlog/spdlog.h>

namespace gl
{
class VertexArray final : public Object<VertexArray> {
public:
    VertexArray() = default;
    VertexArray(VertexArray &&rhs);
    VertexArray &operator=(VertexArray &&rhs);
    void create();
    void destroy();
    void bind() const;
    void setIndexBuffer(const Buffer &ibo);
    void setVertexBuffer(uint32_t binding, const Buffer &vbo, size_t stride);
    void enableAttribute(uint32_t attrib, bool enable);
    void setAttributeFormat(uint32_t attrib, uint32_t type, size_t count, size_t offset, bool normalized);
    void setAttributeBinding(uint32_t attrib, uint32_t binding);
};
} // namespace gl

inline gl::VertexArray::VertexArray(gl::VertexArray &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::VertexArray &gl::VertexArray::operator=(gl::VertexArray &&rhs)
{
    gl::VertexArray copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::VertexArray::create()
{
    destroy();
    glCreateVertexArrays(1, &handle);
}

inline void gl::VertexArray::destroy()
{
    if(handle) {
        glDeleteVertexArrays(1, &handle);
        handle = 0;
    }
}

inline void gl::VertexArray::bind() const
{
    glBindVertexArray(handle);
}

inline void gl::VertexArray::setIndexBuffer(const gl::Buffer &ibo)
{
    glVertexArrayElementBuffer(handle, ibo.get());
}

inline void gl::VertexArray::setVertexBuffer(uint32_t binding, const gl::Buffer &vbo, size_t stride)
{
    glVertexArrayVertexBuffer(handle, binding, vbo.get(), 0, static_cast<GLsizei>(stride));
}

inline void gl::VertexArray::enableAttribute(uint32_t attrib, bool enable)
{
    if(enable) {
        glEnableVertexArrayAttrib(handle, attrib);
        return;
    }

    glDisableVertexArrayAttrib(handle, attrib);
}

inline void gl::VertexArray::setAttributeFormat(uint32_t attrib, uint32_t type, size_t count, size_t offset, bool normalized)
{
    switch(type) {
        case GL_FLOAT:
            glVertexArrayAttribFormat(handle, attrib, static_cast<GLint>(count), type, normalized ? GL_TRUE : GL_FALSE, static_cast<GLuint>(offset));
            break;
        case GL_INT:
        case GL_UNSIGNED_INT:
            glVertexArrayAttribIFormat(handle, attrib, static_cast<GLint>(count), type, static_cast<GLuint>(offset));
            break;
        default:
            spdlog::warn("GL: unknown attrib type {}", type);
            break;
    }
}

inline void gl::VertexArray::setAttributeBinding(uint32_t attrib, uint32_t binding)
{
    glVertexArrayAttribBinding(handle, attrib, binding);
}

