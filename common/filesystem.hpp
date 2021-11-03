/*
 * filesystem.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace stdfs = std::filesystem;
namespace fs
{
void init();
void shutdown();
bool setRoot(const stdfs::path &path);
void mount(const stdfs::path &path);
bool exists(const stdfs::path &path);
bool readBytes(const stdfs::path &path, std::vector<uint8_t> &buffer);
bool writeBytes(const stdfs::path &path, const std::vector<uint8_t> &buffer);
bool readText(const stdfs::path &path, std::string &buffer);
bool writeText(const stdfs::path &path, const std::string &buffer);
const stdfs::path getFullPath(stdfs::path path);
const stdfs::path getWritePath(stdfs::path path);
} // namespace fs
