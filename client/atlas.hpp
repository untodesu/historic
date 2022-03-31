/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/texture.hpp>
#include <common/mixin.hpp>
#include <string>
#include <unordered_map>

struct AtlasEntry final {
    GLuint entry_index;
    vector2f_t max_texcoord;
};

class Atlas final : public mixin::NoCopy, public mixin::NoMove {
public:
    Atlas();

    bool create(unsigned int width, unsigned int height, unsigned int num_layers);
    void destroy();
    void submit();

    const AtlasEntry *push(const std::string &path);
    const AtlasEntry *find(const std::string &path) const;
    const gl::Texture2DArray &get() const;

private:
    unsigned int width;
    unsigned int height;
    unsigned int num_layers;
    unsigned int head;
    gl::Texture2DArray texture;
    std::unordered_map<std::string, AtlasEntry> entries;
};
