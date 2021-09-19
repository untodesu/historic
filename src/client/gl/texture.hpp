/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/object.hpp>
#include <math/util.hpp>

namespace gl
{
enum class PixelFormat {
    R8_UNORM,
    R8_SINT,
    R8_UINT,
    R8G8_UNORM,
    R8G8_SINT,
    R8G8_UINT,
    R8G8B8_UNORM,
    R8G8B8_SINT,
    R8G8B8_UINT,
    R8G8B8A8_UNORM,
    R8G8B8A8_SINT,
    R8G8B8A8_UINT,
    R16_UNORM,
    R16_SINT,
    R16_UINT,
    R16_FLOAT,
    R16G16_UNORM,
    R16G16_SINT,
    R16G16_UINT,
    R16G16_FLOAT,
    R16G16B16_UNORM,
    R16G16B16_SINT,
    R16G16B16_UINT,
    R16G16B16_FLOAT,
    R16G16B16A16_UNORM,
    R16G16B16A16_SINT,
    R16G16B16A16_UINT,
    R16G16B16A16_FLOAT,
    R32_SINT,
    R32_UINT,
    R32_FLOAT,
    R32G32_SINT,
    R32G32_UINT,
    R32G32_FLOAT,
    R32G32B32_SINT,
    R32G32B32_UINT,
    R32G32B32_FLOAT,
    R32G32B32A32_SINT,
    R32G32B32A32_UINT,
    R32G32B32A32_FLOAT,
    D16_UNORM,
    D32_FLOAT,
    S8_UINT,
};

class Texture : public Object<Texture> {
public:
    Texture(uint32_t target);
    void create();
    void destroy();
    void genMipmap();
    void bind(uint32_t unit) const;

protected:
    uint32_t target;
};

class Texture2D final : public Texture {
public:
    Texture2D();
    Texture2D(Texture2D &&rhs);
    Texture2D &operator=(Texture2D &&rhs);
    bool storage(int width, int height, PixelFormat format);
    bool write(int x, int y, int width, int height, PixelFormat format, const void *data);
};

class Texture2DArray final : public Texture {
public:
    Texture2DArray();
    Texture2DArray(Texture2DArray &&rhs);
    Texture2DArray &operator=(Texture2DArray &&rhs);
    bool storage(int width, int height, int layers, PixelFormat format);
    bool write(int layer, int x, int y, int width, int height, PixelFormat format, const void *data);
};
} // namespace gl

namespace detail
{
static inline uint32_t getInternalFormat(gl::PixelFormat format)
{
    switch(format) {
        case gl::PixelFormat::R8_UNORM:
            return GL_R8;
        case gl::PixelFormat::R8_SINT:
            return GL_R8I;
        case gl::PixelFormat::R8_UINT:
            return GL_R8UI;
        case gl::PixelFormat::R8G8_UNORM:
            return GL_RG8;
        case gl::PixelFormat::R8G8_SINT:
            return GL_RG8I;
        case gl::PixelFormat::R8G8_UINT:
            return GL_RG8UI;
        case gl::PixelFormat::R8G8B8_UNORM:
            return GL_RGB8;
        case gl::PixelFormat::R8G8B8_SINT:
            return GL_RGB8I;
        case gl::PixelFormat::R8G8B8_UINT:
            return GL_RGB8UI;
        case gl::PixelFormat::R8G8B8A8_UNORM:
            return GL_RGBA8;
        case gl::PixelFormat::R8G8B8A8_SINT:
            return GL_RGBA8I;
        case gl::PixelFormat::R8G8B8A8_UINT:
            return GL_RGBA8UI;
        case gl::PixelFormat::R16_UNORM:
            return GL_R16;
        case gl::PixelFormat::R16_SINT:
            return GL_R16I;
        case gl::PixelFormat::R16_UINT:
            return GL_R16UI;
        case gl::PixelFormat::R16_FLOAT:
            return GL_R16F;
        case gl::PixelFormat::R16G16_UNORM:
            return GL_RG16;
        case gl::PixelFormat::R16G16_SINT:
            return GL_RG16I;
        case gl::PixelFormat::R16G16_UINT:
            return GL_RG16UI;
        case gl::PixelFormat::R16G16_FLOAT:
            return GL_RG16F;
        case gl::PixelFormat::R16G16B16_UNORM:
            return GL_RGB16;
        case gl::PixelFormat::R16G16B16_SINT:
            return GL_RGB16I;
        case gl::PixelFormat::R16G16B16_UINT:
            return GL_RGB16UI;
        case gl::PixelFormat::R16G16B16_FLOAT:
            return GL_RGB16F;
        case gl::PixelFormat::R16G16B16A16_UNORM:
            return GL_RGBA16;
        case gl::PixelFormat::R16G16B16A16_SINT:
            return GL_RGBA16I;
        case gl::PixelFormat::R16G16B16A16_UINT:
            return GL_RGBA16UI;
        case gl::PixelFormat::R16G16B16A16_FLOAT:
            return GL_RGBA16F;
        case gl::PixelFormat::R32_SINT:
            return GL_R32I;
        case gl::PixelFormat::R32_UINT:
            return GL_R32UI;
        case gl::PixelFormat::R32_FLOAT:
            return GL_R32F;
        case gl::PixelFormat::R32G32_SINT:
            return GL_RG32I;
        case gl::PixelFormat::R32G32_UINT:
            return GL_RG32UI;
        case gl::PixelFormat::R32G32_FLOAT:
            return GL_RG32F;
        case gl::PixelFormat::R32G32B32_SINT:
            return GL_RGB32I;
        case gl::PixelFormat::R32G32B32_UINT:
            return GL_RGB32UI;
        case gl::PixelFormat::R32G32B32_FLOAT:
            return GL_RGB32F;
        case gl::PixelFormat::R32G32B32A32_SINT:
            return GL_RGBA32I;
        case gl::PixelFormat::R32G32B32A32_UINT:
            return GL_RGBA32UI;
        case gl::PixelFormat::R32G32B32A32_FLOAT:
            return GL_RGBA32F;
        case gl::PixelFormat::D16_UNORM:
            return GL_DEPTH_COMPONENT16;
        case gl::PixelFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;
        case gl::PixelFormat::S8_UINT:
            return GL_STENCIL_INDEX8;
        default:
            return 0;
    }
}

static inline bool getExternalFormat(gl::PixelFormat format, uint32_t &fmt, uint32_t &type)
{
    switch(format) {
        case gl::PixelFormat::R8_UNORM:
        case gl::PixelFormat::R8_SINT:
        case gl::PixelFormat::R8_UINT:
        case gl::PixelFormat::R16_UNORM:
        case gl::PixelFormat::R16_SINT:
        case gl::PixelFormat::R16_UINT:
        case gl::PixelFormat::R16_FLOAT:
        case gl::PixelFormat::R32_SINT:
        case gl::PixelFormat::R32_UINT:
        case gl::PixelFormat::R32_FLOAT:
            fmt = GL_RED;
            break;
        case gl::PixelFormat::R8G8_UNORM:
        case gl::PixelFormat::R8G8_SINT:
        case gl::PixelFormat::R8G8_UINT:
        case gl::PixelFormat::R16G16_UNORM:
        case gl::PixelFormat::R16G16_SINT:
        case gl::PixelFormat::R16G16_UINT:
        case gl::PixelFormat::R16G16_FLOAT:
        case gl::PixelFormat::R32G32_SINT:
        case gl::PixelFormat::R32G32_UINT:
        case gl::PixelFormat::R32G32_FLOAT:
            fmt = GL_RG;
            break;
        case gl::PixelFormat::R8G8B8_UNORM:
        case gl::PixelFormat::R8G8B8_SINT:
        case gl::PixelFormat::R8G8B8_UINT:
        case gl::PixelFormat::R16G16B16_UNORM:
        case gl::PixelFormat::R16G16B16_SINT:
        case gl::PixelFormat::R16G16B16_UINT:
        case gl::PixelFormat::R16G16B16_FLOAT:
        case gl::PixelFormat::R32G32B32_SINT:
        case gl::PixelFormat::R32G32B32_UINT:
        case gl::PixelFormat::R32G32B32_FLOAT:
            fmt = GL_RGB;
            break;
        case gl::PixelFormat::R8G8B8A8_UNORM:
        case gl::PixelFormat::R8G8B8A8_SINT:
        case gl::PixelFormat::R8G8B8A8_UINT:
        case gl::PixelFormat::R16G16B16A16_UNORM:
        case gl::PixelFormat::R16G16B16A16_SINT:
        case gl::PixelFormat::R16G16B16A16_UINT:
        case gl::PixelFormat::R16G16B16A16_FLOAT:
        case gl::PixelFormat::R32G32B32A32_SINT:
        case gl::PixelFormat::R32G32B32A32_UINT:
        case gl::PixelFormat::R32G32B32A32_FLOAT:
            fmt = GL_RGBA;
            break;
        default:
            return false;
    }

    switch(format) {
        case gl::PixelFormat::R8_SINT:
        case gl::PixelFormat::R8G8_SINT:
        case gl::PixelFormat::R8G8B8_SINT:
        case gl::PixelFormat::R8G8B8A8_SINT:
            type = GL_BYTE;
            break;
        case gl::PixelFormat::R8_UNORM:
        case gl::PixelFormat::R8_UINT:
        case gl::PixelFormat::R8G8_UNORM:
        case gl::PixelFormat::R8G8_UINT:
        case gl::PixelFormat::R8G8B8_UNORM:
        case gl::PixelFormat::R8G8B8_UINT:
        case gl::PixelFormat::R8G8B8A8_UNORM:
        case gl::PixelFormat::R8G8B8A8_UINT:
            type = GL_UNSIGNED_BYTE;
            break;
        case gl::PixelFormat::R16_SINT:
        case gl::PixelFormat::R16G16_SINT:
        case gl::PixelFormat::R16G16B16_SINT:
        case gl::PixelFormat::R16G16B16A16_SINT:
            type = GL_SHORT;
            break;
        case gl::PixelFormat::R16_UNORM:
        case gl::PixelFormat::R16_UINT:
        case gl::PixelFormat::R16G16_UNORM:
        case gl::PixelFormat::R16G16_UINT:
        case gl::PixelFormat::R16G16B16_UNORM:
        case gl::PixelFormat::R16G16B16_UINT:
        case gl::PixelFormat::R16G16B16A16_UNORM:
        case gl::PixelFormat::R16G16B16A16_UINT:
            type = GL_UNSIGNED_SHORT;
            break;
        case gl::PixelFormat::R32_SINT:
        case gl::PixelFormat::R32G32_SINT:
        case gl::PixelFormat::R32G32B32_SINT:
        case gl::PixelFormat::R32G32B32A32_SINT:
            type = GL_INT;
            break;
        case gl::PixelFormat::R32_UINT:
        case gl::PixelFormat::R32G32_UINT:
        case gl::PixelFormat::R32G32B32_UINT:
        case gl::PixelFormat::R32G32B32A32_UINT:
            type = GL_UNSIGNED_INT;
            break;
        case gl::PixelFormat::R32_FLOAT:
        case gl::PixelFormat::R32G32_FLOAT:
        case gl::PixelFormat::R32G32B32_FLOAT:
        case gl::PixelFormat::R32G32B32A32_FLOAT:
            type = GL_FLOAT;
            break;
        default:
            return false;
    }

    return true;
}
} // namespace detail

inline gl::Texture::Texture(uint32_t target)
    : target(target)
{

}

inline void gl::Texture::create()
{
    destroy();
    glCreateTextures(target, 1, &handle);
}

inline void gl::Texture::destroy()
{
    if(handle) {
        glDeleteTextures(1, &handle);
        handle = 0;
    }
}

inline void gl::Texture::genMipmap()
{
    glGenerateTextureMipmap(handle);
}

inline void gl::Texture::bind(uint32_t unit) const
{
    glBindTextureUnit(unit, handle);
}

inline gl::Texture2D::Texture2D()
    : gl::Texture(GL_TEXTURE_2D)
{
}

inline gl::Texture2D::Texture2D(gl::Texture2D &&rhs)
    : gl::Texture(GL_TEXTURE_2D)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Texture2D &gl::Texture2D::operator=(gl::Texture2D &&rhs)
{
    gl::Texture2D copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline bool gl::Texture2D::storage(int width, int height, gl::PixelFormat format)
{
    if(uint32_t f = detail::getInternalFormat(format)) {
        glTextureStorage2D(handle, math::log2<int>(glm::max(width, height)), f, width, height);
        return true;
    }

    return false;
}

inline bool gl::Texture2D::write(int x, int y, int width, int height, gl::PixelFormat format, const void *data)
{
    uint32_t fmt, type;
    if(detail::getExternalFormat(format, fmt, type)) {
        glTextureSubImage2D(handle, 0, x, y, width, height, fmt, type, data);
        return true;
    }

    return false;
}

inline gl::Texture2DArray::Texture2DArray()
    : gl::Texture(GL_TEXTURE_2D_ARRAY)
{
}

inline gl::Texture2DArray::Texture2DArray(gl::Texture2DArray &&rhs)
    : gl::Texture(GL_TEXTURE_2D_ARRAY)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline gl::Texture2DArray &gl::Texture2DArray::operator=(gl::Texture2DArray &&rhs)
{
    gl::Texture2DArray copy(std::move(rhs));
    std::swap(handle, copy.handle);
    return *this;
}

inline bool gl::Texture2DArray::storage(int width, int height, int layers, gl::PixelFormat format)
{
    if(uint32_t f = detail::getInternalFormat(format)) {
        glTextureStorage3D(handle, math::log2<int>(glm::max(width, height)), f, width, height, layers);
        return true;
    }

    return false;
}

inline bool gl::Texture2DArray::write(int layer, int x, int y, int width, int height, PixelFormat format, const void *data)
{
    uint32_t fmt, type;
    if(detail::getExternalFormat(format, fmt, type)) {
        glTextureSubImage3D(handle, 0, x, y, layer, width, height, 1, fmt, type, data);
        return true;
    }

    return false;
}
