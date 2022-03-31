/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/globals.hpp>
#include <client/image.hpp>
#include <common/math/constexpr.hpp>
#include <spdlog/spdlog.h>

Atlas::Atlas()
    : width(0), height(0), num_layers(0), head(0)
{

}

bool Atlas::create(unsigned int width, unsigned int height, unsigned int num_layers)
{
    destroy();

    this->width = width;
    this->height = height;
    this->num_layers = num_layers;
    this->head = 0;

    texture.create();
    if(!texture.storage(static_cast<int>(this->width), static_cast<int>(this->height), static_cast<int>(this->num_layers), Image::FORMAT)) {
        spdlog::error("atlas: unable to create/storage a texture array");
        destroy();
        return false;
    }

    return true;
}

void Atlas::destroy()
{
    width = 0;
    height = 0;
    num_layers = 0;
    head = std::numeric_limits<unsigned int>::max();
    texture.destroy();
}

void Atlas::submit()
{
    // FIXME: should we generate a mipmap for
    // the atlas considering we have no filtering?
    // texture.genMipmap();
}

const AtlasEntry *Atlas::push(const std::string &path)
{
    if(!texture.valid()) {
        spdlog::warn("atlas: texture array is not valid");
        return nullptr;
    }

    const auto it = entries.find(path);
    if(it != entries.cend()) {
        // FIXME: it FLOODS the logs
        // spdlog::warn("atlas: {} is already present", path);
        return &it->second;
    }

    if(head > num_layers) {
        spdlog::warn("atlas: num_layers exceeded ({} > {})", head, num_layers);
        return nullptr;
    }

    Image image;
    if(image.load(path)) {
        int image_w, image_h;
        image.size(image_w, image_h);
        image_w = math::clamp<int>(image_w, 0, width);
        image_h = math::clamp<int>(image_h, 0, height);

        AtlasEntry entry = {};
        entry.entry_index = static_cast<GLuint>(head++);
        entry.max_texcoord.x = static_cast<float>(image_w) / static_cast<float>(width);
        entry.max_texcoord.y = static_cast<float>(image_h) / static_cast<float>(height);

        texture.write(static_cast<int>(entry.entry_index), 0, 0, image_w, image_h, Image::FORMAT, image.data());

        return &(entries[path] = entry);
    }

    spdlog::warn("atlas: unable to load {}", path);

    return false;
}

const AtlasEntry *Atlas::find(const std::string &path) const
{
    const auto it = entries.find(path);
    if(it != entries.cend())
        return &it->second;
    return nullptr;
}

const gl::Texture2DArray &Atlas::get() const
{
    return texture;
}
