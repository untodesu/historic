/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/fs.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sstream>
#include <toml.hpp>

template<typename impl_type>
class TomlConfig {
public:
    bool read(const fs_std::path &path);
    void write(const fs_std::path &path);

    // implementations define:
    //  void impl_postRead();
    //  void impl_preWrite();

public:
    toml::table toml;
};

template<typename impl_type>
inline bool TomlConfig<impl_type>::read(const fs_std::path &path)
{
    bool success = true;

    try {
        std::string source;
        if(!fs::readString(path, source))
            throw std::runtime_error("unable to read file");
        toml = toml::parse(source);
    }
    catch(const std::exception &ex) {
        spdlog::error("config: parsing {} failed: {}", path.string(), ex.what());
        toml = toml::table();
        success = false;
    }

    static_cast<impl_type *>(this)->impl_postRead();

    return success;
}

template<typename impl_type>
inline void TomlConfig<impl_type>::write(const fs_std::path &path)
{
    static_cast<impl_type *>(this)->impl_preWrite();
    std::stringstream ss;
    ss << toml;
    fs::writeString(path, ss.str());
}
