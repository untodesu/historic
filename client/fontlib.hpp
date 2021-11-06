/*
 * fontlib.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>
#include <common/traits.hpp>
#include <imgui.h>

namespace fontlib
{
namespace detail
{
class FontFileBuilder;
class FontBuilder final : public NonCopyable, public NonMovable {
    friend class FontFileBuilder;

public:
    FontBuilder(const std::string &id);
    FontFileBuilder file(const stdfs::path &path);
    ImFont *submit();

private:
    ImFont *font;
    std::string id;
};

class FontFileBuilder final : public NonCopyable, public NonMovable {
public:
    FontFileBuilder(FontBuilder *parent, const stdfs::path &path);
    FontFileBuilder &brightness(float brightness_factor);
    FontFileBuilder &range(ImWchar begin, ImWchar end);
    FontFileBuilder &range(const ImWchar *ptr);
    FontFileBuilder &size(float size_px);
    FontBuilder &endFile();

private:
    FontBuilder *parent;
    stdfs::path path;
    float brightness_factor;
    ImWchar glyph_range[3];
    const ImWchar *glyph_range_p;
    float size_px;
};
} // namespace detail

void init();
void shutdown();
detail::FontBuilder build(const std::string &id);
ImFont *find(const std::string &id);
} // namespace fontlib
