/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/mixin.hpp>
#include <common/types.hpp>
#include <glad/gl.h>

namespace gl
{
template<typename object_type>
class Object : public mixin::NoCopy {
public:
    Object() = default;
    virtual ~Object();
    void create();
    void destroy();
    constexpr bool valid() const;
    constexpr GLuint get() const;

protected:
    GLuint handle {0};
};
} // namespace gl

template<typename object_type>
inline gl::Object<object_type>::~Object()
{
    destroy();
}

template<typename object_type>
inline void gl::Object<object_type>::create()
{
    static_cast<object_type *>(this)->create();
}

template<typename object_type>
inline void gl::Object<object_type>::destroy()
{
    static_cast<object_type *>(this)->destroy();
}

template<typename object_type>
inline constexpr bool gl::Object<object_type>::valid() const
{
    return handle != 0;
}

template<typename object_type>
inline constexpr GLuint gl::Object<object_type>::get() const
{
    return handle;
}
