/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs_std = std::filesystem;
namespace fs
{
using buffer_type = std::vector<uint8_t>;
using string_type = std::string;

void initialize();
void shutdown();

// search path api a.k.a. vfs apple core
const fs_std::path getRWPath(const fs_std::path &path);
const fs_std::path getROPath(const fs_std::path &path);
bool addSearchPath(const fs_std::path &path);
bool setSearchRoot(const fs_std::path &path);

// file read/write utils via search paths
bool readBinary(const fs_std::path &path, buffer_type &out);
bool readString(const fs_std::path &path, string_type &out);
bool writeBinary(const fs_std::path &path, const buffer_type &in);
bool writeString(const fs_std::path &path, const string_type &in);
} // namespace fs
