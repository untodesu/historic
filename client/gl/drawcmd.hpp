/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/object.hpp>
#include <spdlog/spdlog.h>

namespace gl
{
struct DrawArraysCmd final {
    GLuint vertices;
    GLuint instances;
    GLuint base_vertex;
    GLuint base_instance;
};

struct DrawElementsCmd final {
    GLuint indices;
    GLuint instances;
    GLuint base_index;
    GLuint base_vertex;
    GLuint base_instance;
};

class DrawCommand final : public Object<DrawCommand> {
public:
    DrawCommand() = default;
    DrawCommand(GLenum mode, size_t vertices, size_t instances, size_t base_vertex, size_t base_instance);
    DrawCommand(GLenum mode, GLenum type, size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance);
    DrawCommand(DrawCommand &&rhs);
    DrawCommand &operator=(DrawCommand &&rhs);
    void create();
    void destroy();
    void set(GLenum mode, size_t vertices, size_t instances, size_t base_vertex, size_t base_instance);
    void set(GLenum mode, GLenum type, size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance);
    void invoke();
    size_t size() const;

private:
    bool indexed {false};
    GLenum mode {0};
    GLenum type {0};
    size_t nv {0};
};
} // namespace gl

inline gl::DrawCommand::DrawCommand(GLenum mode, size_t vertices, size_t instances, size_t base_vertex, size_t base_instance)
    : indexed(false), mode(mode), type(0), nv(0)
{
    create();
    set(mode, vertices, instances, base_vertex, base_instance);
}

inline gl::DrawCommand::DrawCommand(GLenum mode, GLenum type, size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance)
    : indexed(true), mode(mode), type(type), nv(0)
{
    create();
    set(mode, type, indices, instances, base_index, base_vertex, base_instance);
}

inline gl::DrawCommand::DrawCommand(gl::DrawCommand &&rhs)
{
    handle = rhs.handle;
    indexed = rhs.indexed;
    mode = rhs.mode;
    type = rhs.type;
    nv = rhs.nv;
    rhs.handle = 0;
    rhs.indexed = false;
    rhs.mode = 0;
    rhs.type = 0;
    rhs.nv = 0;
}

inline gl::DrawCommand &gl::DrawCommand::operator=(gl::DrawCommand &&rhs)
{
    gl::DrawCommand copy(std::move(rhs));
    std::swap(handle, copy.handle);
    std::swap(indexed, rhs.indexed);
    std::swap(mode, rhs.mode);
    std::swap(type, rhs.type);
    std::swap(nv, rhs.nv);
    return *this;
}

inline void gl::DrawCommand::create()
{
    destroy();
    glCreateBuffers(1, &handle);
}

inline void gl::DrawCommand::destroy()
{
    if(handle) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }
}

inline void gl::DrawCommand::set(GLenum mode, size_t vertices, size_t instances, size_t base_vertex, size_t base_instance)
{
    nv = vertices;
    indexed = false;
    this->mode = mode;
    gl::DrawArraysCmd cmd = {};
    cmd.vertices = static_cast<GLuint>(vertices);
    cmd.instances = static_cast<GLuint>(instances);
    cmd.base_vertex = static_cast<GLuint>(base_vertex);
    cmd.base_instance = static_cast<GLuint>(base_instance);
    glNamedBufferData(handle, static_cast<GLsizeiptr>(sizeof(cmd)), &cmd, GL_STATIC_DRAW);
}

inline void gl::DrawCommand::set(GLenum mode, GLenum type, size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance)
{
    nv = indices;
    indexed = true;
    this->mode = mode;
    this->type = type;
    gl::DrawElementsCmd cmd = {};
    cmd.indices = static_cast<GLuint>(indices);
    cmd.instances = static_cast<GLuint>(instances);
    cmd.base_index = static_cast<GLuint>(base_index);
    cmd.base_vertex = static_cast<GLuint>(base_vertex);
    cmd.base_instance = static_cast<GLuint>(base_instance);
    glNamedBufferData(handle, static_cast<GLsizeiptr>(sizeof(cmd)), &cmd, GL_STATIC_DRAW);
}

inline void gl::DrawCommand::invoke()
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, handle);
    
    if(indexed) {
        glDrawElementsIndirect(mode, type, nullptr);
        return;
    }

    glDrawArraysIndirect(mode, nullptr);
}

inline size_t gl::DrawCommand::size() const
{
    return nv;
}
