/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/pixel_format.hpp>
#include <common/fs.hpp>

class Image final {
public:
    constexpr static const gl::PixelFormat FORMAT = gl::PixelFormat::R8G8B8A8_UNORM;

public:
    Image();
    Image(const fs_std::path &path);
    Image(const Image &rhs) = delete;
    Image(Image &&rhs);
    virtual ~Image();

    Image &operator=(const Image &rhs) = delete;
    Image &operator=(Image &&rhs);

    bool load(const fs_std::path &path);
    bool valid() const;
    void size(int &width, int &height) const;
    const void *data() const;

private:
    int width, height;
    void *pixels;
};
