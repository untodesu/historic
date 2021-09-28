/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/util/image.hpp>
#include <stb_image.h>

Image::Image()
    : width(0), height(0), pixels(nullptr)
{

}

Image::Image(const stdfs::path &path)
{
    load(path);
}

Image::Image(Image &&rhs)
    : width(rhs.width), height(rhs.height), pixels(rhs.pixels)
{
    rhs.width = 0;
    rhs.height = 0;
    rhs.pixels = nullptr;
}

Image::~Image()
{
    if(!pixels)
        return;
    stbi_image_free(pixels);
}

Image &Image::operator=(Image &&rhs)
{
    Image copy(std::move(rhs));
    std::swap(copy.width, width);
    std::swap(copy.height, height);
    std::swap(copy.pixels, pixels);
    return *this;
}

bool Image::load(const stdfs::path &path)
{
    stbi_set_flip_vertically_on_load(1);

    if(pixels) {
        stbi_image_free(pixels);
        pixels = nullptr;
    }

    std::vector<uint8_t> buffer;
    if(fs::readBytes(path, buffer)) {
        pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(buffer.data()), static_cast<int>(buffer.size()), &width, &height, nullptr, STBI_rgb_alpha);
        return width && height && pixels;
    }

    return false;
}

bool Image::valid() const
{
    return width && height && pixels;
}

void Image::size(int &width, int &height) const
{
    width = this->width;
    height = this->height;
}

const void *Image::data() const
{
    return pixels;
}
