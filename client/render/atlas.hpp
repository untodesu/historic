/*
 * atlas.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <client/render/gl/texture.hpp>
#include <string>
#include <unordered_map>

struct AtlasNode final {
    uint32_t index;
    float2 max_uv;
};

class Atlas final {
public:
    Atlas();

    bool create(uint32_t width, uint32_t height, uint32_t layers);
    void destroy();
    void submit();

    const AtlasNode *push(const std::string &path);
    const AtlasNode *getNode(const std::string &path) const;
    const gl::Texture2DArray &getTexture() const;

private:
    uint32_t w, h, d, head;
    gl::Texture2DArray texture;
    std::unordered_map<std::string, AtlasNode> list;
};
