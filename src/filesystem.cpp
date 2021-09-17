/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <filesystem.hpp>
#include <fstream>
#include <iterator>

static stdfs::path root_path;
static std::vector<stdfs::path> mount_points;

void fs::init()
{
    root_path = stdfs::current_path();
    mount_points.clear();
}

void fs::shutdown()
{
    mount_points.clear();
    root_path.clear();
}

void fs::mount(const stdfs::path &path)
{
    for(const stdfs::path &it : mount_points) {
        if(it == path) {
            // don't allow the same paths
            return;
        }
    }

    // Prepend
    mount_points.insert(mount_points.begin(), path);
}

bool fs::exists(const stdfs::path &path)
{
    return stdfs::exists(fs::getFullPath(path));
}

bool fs::readBytes(const stdfs::path &path, std::vector<uint8_t> &buffer)
{
    std::ifstream ifs(fs::getFullPath(path), std::ios::binary);
    if(!ifs.is_open())
        return false;

    ifs.unsetf(std::ios::skipws);

    ifs.seekg(0, std::ios::end);
    buffer.resize(static_cast<size_t>(ifs.tellg()));
    ifs.seekg(0, std::ios::beg);

    buffer.insert(buffer.begin(), std::istream_iterator<char>(ifs), std::istream_iterator<char>());

    ifs.close();
    return true;
}

bool fs::writeBytes(const stdfs::path &path, const std::vector<uint8_t> &buffer)
{
    std::ofstream ofs(fs::getWritePath(path), std::ios::binary);
    if(!ofs.is_open())
        return false;

    ofs.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());

    ofs.close();
    return true;
}

bool fs::readText(const stdfs::path &path, std::string &buffer)
{
    std::ifstream ifs(fs::getFullPath(path));
    if(!ifs.is_open())
        return false;

    ifs.seekg(0, std::ios::end);
    buffer.resize(static_cast<size_t>(ifs.tellg()));
    ifs.seekg(0, std::ios::beg);

    buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

    ifs.close();
    return true;
}

bool fs::writeText(const stdfs::path &path, const std::string &buffer)
{
    std::ofstream ofs(fs::getWritePath(path));
    if(!ofs.is_open())
        return false;

    ofs << buffer;
    ofs << std::endl;

    ofs.close();
    return true;
}

const stdfs::path fs::getFullPath(stdfs::path path)
{
    for(const stdfs::path &it : mount_points) {
        stdfs::path full_path = it / path;
        if(stdfs::exists(full_path)) {
            // Bullseye
            return full_path;
        }
    }

    // TODO: there should be a better way.
    return root_path / path;
}

const stdfs::path fs::getWritePath(stdfs::path path)
{
    // We can write only in the root directory
    return root_path / path;
}
