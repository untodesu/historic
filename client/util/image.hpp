/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/texture.hpp>
#include <common/filesystem.hpp>

class Image final {
public:
    constexpr static const gl::PixelFormat FORMAT = gl::PixelFormat::R8G8B8A8_UNORM;

public:
    Image();
    Image(const stdfs::path &path);
    Image(const Image &rhs) = delete;
    Image(Image &&rhs);
    virtual ~Image();

    Image &operator=(const Image &rhs) = delete;
    Image &operator=(Image &&rhs);

    bool load(const stdfs::path &path);
    bool valid() const;
    void size(int &width, int &height) const;
    const void *data() const;

private:
    int width, height;
    void *pixels;
};
