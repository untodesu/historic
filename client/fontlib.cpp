/*
 * fontlib.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/fontlib.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

static ImFont *default_font = nullptr;
static std::unordered_map<std::string, ImFont *> fonts;

fontlib::detail::FontBuilder::FontBuilder(const std::string &id)
    : font(nullptr), id(id)
{

}

fontlib::detail::FontFileBuilder fontlib::detail::FontBuilder::file(const stdfs::path &path)
{
    return fontlib::detail::FontFileBuilder(this, path);
}

ImFont *fontlib::detail::FontBuilder::submit()
{
    if(font) {
        if(fonts.find(id) != fonts.cend())
            spdlog::warn("fontlib: overriding {}", id);
        fonts[id] = font;
        return font;
    }

    spdlog::warn("fontlib: submitting empty fonts is not allowed ({})", id);
    return default_font;
}

fontlib::detail::FontFileBuilder::FontFileBuilder(fontlib::detail::FontBuilder *parent, const stdfs::path &path)
    : parent(parent), path(fs::getFullPath(path)), brightness_factor(1.0f), glyph_range { 0x0000, 0x00FF, 0 }, glyph_range_p(glyph_range), size_px(16.0f)
{

}

fontlib::detail::FontFileBuilder &fontlib::detail::FontFileBuilder::brightness(float brightness_factor)
{
    this->brightness_factor = brightness_factor;
    return *this;
}

fontlib::detail::FontFileBuilder &fontlib::detail::FontFileBuilder::range(ImWchar begin, ImWchar end)
{
    glyph_range[0] = begin;
    glyph_range[1] = end;
    glyph_range_p = glyph_range;
    return *this;
}

fontlib::detail::FontFileBuilder &fontlib::detail::FontFileBuilder::range(const ImWchar *ptr)
{
    glyph_range_p = ptr;
    return *this;
}

fontlib::detail::FontFileBuilder &fontlib::detail::FontFileBuilder::size(float size_px)
{
    this->size_px = size_px;
    return *this;
}

fontlib::detail::FontBuilder &fontlib::detail::FontFileBuilder::endFile()
{
    ImFontConfig config;
    config.MergeMode = !!parent->font;
    config.RasterizerMultiply = brightness_factor;
    ImFont *font = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.string().c_str(), size_px, &config, glyph_range_p);
    if(!parent->font)
        parent->font = font;
    return *parent;
}

void fontlib::init()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    default_font = io.Fonts->AddFontDefault();
    fonts.clear();
}

void fontlib::shutdown()
{
    default_font = nullptr;
    fonts.clear();
    ImGui::GetIO().Fonts->Clear();
}

fontlib::detail::FontBuilder fontlib::build(const std::string &id)
{
    return fontlib::detail::FontBuilder(id);
}

ImFont *fontlib::find(const std::string &id)
{
    const auto it = fonts.find(id);
    if(it != fonts.cend())
        return it->second;
    return default_font;
}
