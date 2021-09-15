/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/client_globals.hpp>
#include <shared/res.hpp>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <uvre/uvre.hpp>
#include <fs.hpp>

static res::ResourceList<uvre::Texture> list;

static void destroyTexture(uvre::Texture *texture)
{
        globals::render_device->destroyTexture(texture);
}

template<>
size_t res::cleanup<uvre::Texture>(res::priority_t priority)
{
    return list.cleanup(priority);
}

template<>
std::shared_ptr<uvre::Texture> res::load<uvre::Texture>(const std::string &name, res::priority_t priority)
{
    const hash_t hash = std::hash<std::string>()(name);

    res::Resource<uvre::Texture> *found = list.find(hash);
    if(found) {
        if(found->priority < priority)
            found->priority = priority;
        return found->ptr;
    }

    // textures/{filename}
    const stdfs::path path = stdfs::path("textures") / stdfs::path(name);

    std::vector<uint8_t> buffer;
    if(!fs::readBytes(path, buffer)) {
        spdlog::warn("Unable to read {}", path.string());
        return HASH_ZERO;
    }

    stbi_set_flip_vertically_on_load(1);

    int width, height;
    void *pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(buffer.data()), static_cast<int>(buffer.size()), &width, &height, nullptr, STBI_rgb_alpha);
    if(!pixels) {
        spdlog::warn("Unable to parse {}", path.string());
        return HASH_ZERO;
    }

    uvre::TextureInfo info = {};
    info.type = uvre::TextureType::TEXTURE_2D;
    info.format = uvre::PixelFormat::R8G8B8A8_UNORM;
    info.width = static_cast<uint32_t>(width);
    info.height = static_cast<uint32_t>(height);

    std::shared_ptr<uvre::Texture> texture = globals::render_device.createSharedTexture(info);
    if(!texture) {
        stbi_image_free(pixels);
        spdlog::warn("Unable to create a texture object for {}", path.string());
        return HASH_ZERO;
    }

    globals::render_device->writeTexture2D(texture.get(), 0, 0, width, height, uvre::PixelFormat::R8G8B8A8_UNORM, pixels);
    stbi_image_free(pixels);

    res::Resource<uvre::Texture> resource = {};
    resource.hash = hash;
    resource.priority = priority;
    resource.ptr = texture;

    list.data.push_back(std::move(resource));

    return texture;
}

template<>
std::shared_ptr<uvre::Texture> res::find<uvre::Texture>(const std::string &name, bool complain)
{
    const hash_t hash = std::hash<std::string>()(name);
    res::Resource<uvre::Texture> *resource = list.find(hash);
    if(!resource) {
        if(complain)
            spdlog::warn("Unable to find {}", name);
        return nullptr;
    }

    return resource->ptr;
}
