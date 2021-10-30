/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/render/gl/object.hpp>
#include <string>
#include <spdlog/spdlog.h>
#include <vector>

namespace gl
{
class Shader final : public Object<Shader> {
public:
    Shader() = default;
    Shader(Shader &&rhs);
    Shader &operator=(Shader &&rhs);
    void create();
    void destroy();
    bool glsl(uint32_t stage, const std::string &source);
    bool spirv(uint32_t stage, const std::vector<uint8_t> &binary);
    constexpr uint32_t stageBit() const;

private:
    uint32_t bit { 0 };
};
} // namespace gl

namespace detail
{
static inline uint32_t getStageBit(uint32_t stage)
{
    if(stage == GL_VERTEX_SHADER)
        return GL_VERTEX_SHADER_BIT;
    if(stage == GL_FRAGMENT_SHADER)
        return GL_FRAGMENT_SHADER_BIT;
    return 0;
}

static inline void checkShaderInfoLog(uint32_t shader)
{
    int32_t length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        std::string info_log(static_cast<size_t>(length) + 1, static_cast<char>(0));
        glGetShaderInfoLog(shader, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
        spdlog::debug(info_log);
    }
}

static inline void checkProgramInfoLog(uint32_t program)
{
    int32_t length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        std::string info_log(static_cast<size_t>(length) + 1, static_cast<char>(0));
        glGetProgramInfoLog(program, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
        spdlog::debug(info_log);
    }
}
} // namespace detail

inline gl::Shader::Shader(gl::Shader &&rhs)
    : bit(rhs.bit)
{
    handle = rhs.handle;
    rhs.handle = 0;
    rhs.bit = 0;
}

inline gl::Shader &gl::Shader::operator=(gl::Shader &&rhs)
{
    gl::Shader copy(std::move(rhs));
    std::swap(handle, copy.handle);
    std::swap(bit, copy.bit);
    return *this;
}

inline void gl::Shader::create()
{
    destroy();
    handle = glCreateProgram();
    glProgramParameteri(handle, GL_PROGRAM_SEPARABLE, GL_TRUE);
}

inline void gl::Shader::destroy()
{
    if(handle) {
        glDeleteProgram(handle);
        handle = 0;
    }
}

inline bool gl::Shader::glsl(uint32_t stage, const std::string &source)
{
    if(!bit && (bit = detail::getStageBit(stage))) {
        int32_t status;
        const char *data = source.c_str();

        uint32_t shader = glCreateShader(stage);
        glShaderSource(shader, 1, &data, nullptr);
        glCompileShader(shader);
        detail::checkShaderInfoLog(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(!status) {
            glDeleteShader(shader);
            return false;
        }

        glAttachShader(handle, shader);
        glLinkProgram(handle);
        glDeleteShader(shader);
        detail::checkProgramInfoLog(handle);

        glGetProgramiv(handle, GL_LINK_STATUS, &status);
        return !!status;
    }

    return false;
}

inline bool gl::Shader::spirv(uint32_t stage, const std::vector<uint8_t> &binary)
{
    if(!bit && (bit = detail::getStageBit(stage))) {
        int32_t status;

        uint32_t shader = glCreateShader(stage);
        glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, binary.data(), static_cast<GLsizei>(binary.size()));
        glSpecializeShader(shader, "main", 0, nullptr, nullptr);
        detail::checkShaderInfoLog(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(!status) {
            glDeleteShader(shader);
            return false;
        }

        glAttachShader(handle, shader);
        glLinkProgram(handle);
        glDeleteShader(shader);
        detail::checkProgramInfoLog(handle);

        glGetProgramiv(handle, GL_LINK_STATUS, &status);
        return !!status;
    }

    return false;
}

inline constexpr uint32_t gl::Shader::stageBit() const
{
    return bit;
}

