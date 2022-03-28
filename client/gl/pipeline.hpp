/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/shader.hpp>

namespace gl
{
class Pipeline : public Object<Pipeline> {
public:
    Pipeline() = default;
    Pipeline(Pipeline &&rhs);
    Pipeline &operator=(Pipeline &&rhs);
    void create();
    void destroy();
    void stage(const Shader &shader);
    void bind() const;
};
} // namespace gl

inline gl::Pipeline::Pipeline(gl::Pipeline &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Pipeline &gl::Pipeline::operator=(gl::Pipeline &&rhs)
{
    gl::Pipeline copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline void gl::Pipeline::create()
{
    destroy();
    glCreateProgramPipelines(1, &handle);
}

inline void gl::Pipeline::destroy()
{
    if(handle) {
        glDeleteProgramPipelines(1, &handle);
        handle = 0;
    }
}

inline void gl::Pipeline::stage(const gl::Shader &shader)
{
    glUseProgramStages(handle, shader.stageBit(), shader.get());
}

inline void gl::Pipeline::bind() const
{
    glBindProgramPipeline(handle);
}
