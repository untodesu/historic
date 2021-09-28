/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/filesystem.hpp>
#include <functional>
#include <glad/gl.h>
#include <game/client/util/screenshots.hpp>
#include <game/client/screen.hpp>
#include <stb_image_write.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

static inline const std::string getScreenshotFilename(const std::string &ext)
{
    const time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d_%H.%M.%S");
    ss << ext;
    return ss.str();
}

void screenshots::jpeg(int quality)
{
    int width, height;
    screen::getSize(width, height);

    size_t pixels_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
    uint8_t *pixels = new uint8_t[pixels_size];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // screenshots/{date}_{time}.ext
    const stdfs::path screenshots_dir = stdfs::path("screenshots");
    const stdfs::path path = screenshots_dir / stdfs::path(getScreenshotFilename(".jpg"));
    stdfs::create_directories(screenshots_dir);

    std::ofstream ofs(fs::getWritePath(path), std::ios::binary);
    if(ofs.is_open()) {
        stbi_flip_vertically_on_write(1);
        stbi_write_jpg_to_func([](void *context, void *data, int size) {
            reinterpret_cast<std::ofstream *>(context)->write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
        }, &ofs, width, height, 3, pixels, quality);
        spdlog::info("Screenshot: JPEG (Q = {}), {} KiB", quality, pixels_size / 1024);
        ofs.close();
    }

    delete[] pixels;
}
