/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/fs.hpp>
#include <deque>
#include <iterator>

static std::deque<fs_std::path> search_paths;
static fs_std::path search_root;

void fs::initialize()
{
    search_root = fs_std::current_path();
    search_paths.clear();
}

void fs::shutdown()
{
    search_root.clear();
    search_paths.clear();
}

const fs_std::path fs::getRWPath(const fs_std::path &path)
{
    return search_root / path;
}

const fs_std::path fs::getROPath(const fs_std::path &path)
{
    for(const fs_std::path &it : search_paths) {
        fs_std::path ro_path = search_root / it / path;
        if(!fs_std::exists(ro_path.parent_path()))
            continue;
        return ro_path;
    }

    return search_root / path;
}

bool fs::addSearchPath(const fs_std::path &path)
{
    for(const fs_std::path &it : search_paths) {
        if(it != path)
            continue;
        return false;
    }

    search_paths.push_front(path);
    return true;
}

bool fs::setSearchRoot(const fs_std::path &path)
{
    const fs_std::path new_root = fs_std::current_path() / path;
    if(!fs_std::is_directory(new_root))
        return false;
    if(!fs_std::exists(new_root) && !fs_std::create_directories(new_root))
        return false;
    search_root = new_root;
    return true;
}

bool fs::readBinary(const fs_std::path &path, fs::buffer_type &out)
{
    std::ifstream ifile = std::ifstream(fs::getROPath(path), std::ios::binary);
    if(!ifile.is_open())
        return false;

    ifile.unsetf(std::ios::skipws);

    ifile.seekg(0, std::ios::end);
    out.resize(static_cast<size_t>(ifile.tellg()));
    ifile.seekg(0, std::ios::beg);

    out.insert(out.begin(), std::istream_iterator<char>(ifile), std::istream_iterator<char>());

    ifile.close();
    return true;
}

bool fs::readString(const fs_std::path &path, fs::string_type &out)
{
    std::ifstream ifile = std::ifstream(fs::getROPath(path));
    if(!ifile.is_open())
        return false;

    ifile.unsetf(std::ios::skipws);

    ifile.seekg(0, std::ios::end);
    out.resize(static_cast<size_t>(ifile.tellg()));
    ifile.seekg(0, std::ios::beg);

    out.assign(std::istreambuf_iterator<char>(ifile), std::istreambuf_iterator<char>());

    ifile.close();
    return true;
}

bool fs::writeBinary(const fs_std::path &path, const fs::buffer_type &in)
{
    std::ofstream ofile = std::ofstream(fs::getRWPath(path), std::ios::binary);
    if(!ofile.is_open())
        return false;
    
    ofile.write(reinterpret_cast<const char *>(in.data()), in.size());
    ofile.close();

    return true;
}

bool fs::writeString(const fs_std::path &path, const fs::string_type &in)
{
    std::ofstream ofile = std::ofstream(fs::getRWPath(path));
    if(!ofile.is_open())
        return false;
    
    ofile << in << std::endl;
    ofile.close();

    return true;
}
