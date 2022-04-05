/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/screen.hpp>
#include <client/screenshots.hpp>
#include <chrono>
#include <common/fs.hpp>
#include <glad/gl.h>
#include <functional>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <stb_image_write.h>
#include <sstream>

static std::string genScreenshotFilename(const std::string &extension)
{
    const time_t curtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&curtime), "%Y-%m-%d_%H.%M.%S");
    ss << extension;
    return ss.str();
}

void screenshots::take()
{
    const vector2i_t size = screen::size2i();
    const size_t buffer_size = static_cast<size_t>(size.x) * static_cast<size_t>(size.y) * 3 / sizeof(GLubyte);

    GLubyte *buffer = new GLubyte[buffer_size];
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    const fs_std::path subdirectory = fs::getRWPath("screenshots");
    const fs_std::path filename = fs_std::path(genScreenshotFilename(".png"));
    const fs_std::path full_path = subdirectory / filename;

    fs_std::create_directories(subdirectory);

    std::ofstream ofile = std::ofstream(fs::getRWPath(full_path), std::ios::binary);
    if(ofile.is_open()) {
        stbi_flip_vertically_on_write(1);
        stbi_write_png_to_func([](void *ctx, void *data, int size) {
            reinterpret_cast<std::ofstream *>(ctx)->write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
        }, &ofile, size.x, size.y, 3, buffer, 3 * size.x);
        spdlog::info("screenshots: wrote {} ({:.2f} KiB)", filename.string(), static_cast<float>(ofile.tellp()) / 1024.0f);
        ofile.close();
    }

    delete[] buffer;
}
