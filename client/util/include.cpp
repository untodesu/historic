/*
 * glslpp.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/util/include.hpp>
#include <common/util/format.hpp>
#include <stdio.h>

bool util::include(const stdfs::path &path, std::string &out, const std::string &comment)
{
    out.clear();

    std::ifstream ifs = std::ifstream(fs::getFullPath(path));
    if(!ifs.is_open())
        return false;

    std::string line;
    while(std::getline(ifs, line)) {
        // I use sscanf here because it's a bit more
        // convenient to have the formatted input instead
        // of this earse(), insert() and find() fuckery.
        // Also I just wrote a literal assembler using sscanf.
        char filename_str[128] = { 0 };
        if(sscanf(line.c_str(), " #include \"%127[^, \"\t\r\n]\"", filename_str) == 1) {
            if(!util::include(path.parent_path() / filename_str, out))
                out += util::format("%s include failed: %s\r\n", comment.c_str(), filename_str);
            continue;
        }

        out += line;
        out += "\r\n";
    }

    return true;
}
