/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/buffer.hpp>
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
    void setVertexBuffer(GLuint binding, const Buffer &vbo, size_t stride);
    void enableAttribute(GLuint attrib, bool enable);
    void setAttributeFormat(GLuint attrib, GLenum type, size_t count, size_t offset, bool normalized);
    void setAttributeBinding(GLuint attrib, GLuint binding);
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

inline void gl::VertexArray::setVertexBuffer(GLuint binding, const gl::Buffer &vbo, size_t stride)
{
    glVertexArrayVertexBuffer(handle, binding, vbo.get(), 0, static_cast<GLsizei>(stride));
}

inline void gl::VertexArray::enableAttribute(GLuint attrib, bool enable)
{
    if(enable) {
        glEnableVertexArrayAttrib(handle, attrib);
        return;
    }

    glDisableVertexArrayAttrib(handle, attrib);
}

inline void gl::VertexArray::setAttributeFormat(GLuint attrib, GLenum type, size_t count, size_t offset, bool normalized)
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

inline void gl::VertexArray::setAttributeBinding(GLuint attrib, GLuint binding)
{
    glVertexArrayAttribBinding(handle, attrib, binding);
}
