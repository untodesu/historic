/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/util/shaders.hpp>
#include <client/client_globals.hpp>
#include <spdlog/spdlog.h>
#include <uvre/uvre.hpp>

uvre::Shader util::loadShader(const stdfs::path &path, uvre::ShaderFormat format, uvre::ShaderStage stage)
{
    std::string src_buffer;
    std::vector<uint8_t> bin_buffer;
    bool read_successful = false;
    switch(format) {
        case uvre::ShaderFormat::BINARY_SPIRV:
            read_successful = fs::readBytes(path, bin_buffer);
            break;
        case uvre::ShaderFormat::SOURCE_GLSL:
            read_successful = fs::readText(path, src_buffer);
            break;
    }

    if(read_successful) {
        uvre::ShaderInfo info = {};
        info.format = format;
        info.stage = stage;

        switch(format) {
            case uvre::ShaderFormat::BINARY_SPIRV:
                info.code_size = bin_buffer.size();
                info.code = bin_buffer.data();
                break;
            case uvre::ShaderFormat::SOURCE_GLSL:
                info.code = src_buffer.c_str();
                break;
        }

        return globals::render_device->createShader(info);
    }

    spdlog::warn("Unable to read {}", path.string());
    return nullptr;
}
