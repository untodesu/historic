/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/render/atlas.hpp>
#include <client/util/image.hpp>
#include <client/globals.hpp>
#include <spdlog/spdlog.h>

Atlas::Atlas()
    : w(0), h(0), d(0), head(0), texture(), list()
{
}

bool Atlas::create(uint32_t width, uint32_t height, uint32_t layers)
{
    destroy();

    w = width;
    h = height;
    d = layers;
    head = 0;

    texture.create();
    if(!texture.storage(static_cast<int>(width), static_cast<int>(height), static_cast<int>(layers), gl::PixelFormat::R8G8B8A8_UNORM)) {
        spdlog::warn("Atlas: unable to create a texture array.");
        destroy();
        return false;
    }

    return true;
}

void Atlas::destroy()
{
    w = 0;
    h = 0;
    d = 0;
    head = std::numeric_limits<uint32_t>::max();
    texture.destroy();
}

void Atlas::submit()
{
    texture.genMipmap();
}

const AtlasNode *Atlas::push(const std::string &path)
{
    if(!texture.valid()) {
        spdlog::warn("Atlas: cannot push() while destroyed.");
        return nullptr;
    }

    const auto it = list.find(path);
    if(it != list.cend()) {
        // It FLOODS
        // spdlog::warn("Atlas: {} is already present in the atlas.", path);
        return &it->second;
    }

    if(head >= d) {
        spdlog::warn("Atlas: maximum limit of {} layers exceeded.", d);
        return nullptr;
    }

    Image image;
    if(image.load(stdfs::path(path))) {
        int img_w, img_h;
        image.size(img_w, img_h);
        img_w = math::clamp<int>(img_w, 0, w);
        img_h = math::clamp<int>(img_h, 0, w);

        AtlasNode node = {};
        node.index = head++;
        node.max_uv.x = static_cast<float>(img_w) / static_cast<float>(w);
        node.max_uv.y = static_cast<float>(img_h) / static_cast<float>(h);
        spdlog::info("Image: {}. max_uv = ({}, {})", path, node.max_uv.x, node.max_uv.y);

        texture.write(static_cast<int>(node.index), 0, 0, img_w, img_h, Image::FORMAT, image.data());
        return &(list[path] = node);
    }

    spdlog::warn("Unable to load {}", path);
    return nullptr;
}

const AtlasNode *Atlas::getNode(const std::string &path) const
{
    const auto it = list.find(path);
    if(it != list.cend())
        return &it->second;
    return nullptr;
}

const gl::Texture2DArray &Atlas::getTexture() const
{
    return texture;
}
