/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/util/image.hpp>
#include <client/atlas.hpp>
#include <client/globals.hpp>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>

Atlas::Atlas()
    : w(0), h(0), d(0), head(0), texture(nullptr), list()
{
}

bool Atlas::create(uint32_t width, uint32_t height, uint32_t layers)
{
    destroy();

    w = width;
    h = height;
    d = layers;
    head = 0;

    uvre::TextureInfo info = {};
    info.type = uvre::TextureType::TEXTURE_ARRAY;
    info.format = uvre::PixelFormat::R8G8B8A8_UNORM;
    info.width = static_cast<int>(width);
    info.height = static_cast<int>(height);
    info.depth = static_cast<int>(layers);

    texture = globals::render_device->createTexture(info);
    if(!texture) {
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
    texture = nullptr;
}

const AtlasNode *Atlas::push(const std::string &path)
{
    if(!texture) {
        spdlog::warn("Atlas: cannot push() while destroyed.");
        return nullptr;
    }

    const auto it = list.find(path);
    if(it != list.cend()) {
        // FIXME: It FLOODS
        //spdlog::warn("Atlas: {} is already present in the atlas.", path);
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
        node.index = static_cast<uint16_t>(head++);
        node.max_uv.x = static_cast<float>(img_w) / static_cast<float>(w);
        node.max_uv.y = static_cast<float>(img_h) / static_cast<float>(h);

        globals::render_device->writeTextureArray(texture, 0, 0, node.index, img_w, img_h, 1, Image::FORMAT, image.data());
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

uvre::Texture Atlas::getTexture() const
{
    return texture;
}
