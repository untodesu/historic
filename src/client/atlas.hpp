/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/types.hpp>
#include <string>
#include <unordered_map>
#include <uvre/fwd.hpp>

struct AtlasNode final {
    uint32_t index;
    float2_t max_uv;
};

class Atlas final {
public:
    Atlas();

    bool create(uint32_t width, uint32_t height, uint32_t layers);
    void destroy();

    const AtlasNode *push(const std::string &path);
    const AtlasNode *getNode(const std::string &path) const;
    uvre::Texture getTexture() const;

private:
    uint32_t w, h, d, head;
    uvre::Texture texture;
    std::unordered_map<std::string, AtlasNode> list;
};
