/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <core/cmdline.hpp>
#include <unordered_map>

static std::unordered_map<std::string, std::string> cmdline_map;

void cmdline::initialize(int argc, char **argv)
{
    cmdline_map.clear();
    for(int i = 1; i < argc; i++) {
        if(cmdline::isOption(argv[i])) {
            std::string option;
            cmdline::getOption(argv[i], option);
            if(i + 1 < argc && !cmdline::isOption(argv[i + 1])) {
                cmdline::push(option, argv[++i]);
                continue;
            }

            cmdline::push(option);
        }
    }
}

void cmdline::shutdown()
{
    cmdline_map.clear();
}

bool cmdline::isOption(const std::string &argv)
{
    if(argv.find_last_of('-') >= argv.size() - 1)
        return false;
    return argv[0] == '-';
}

void cmdline::getOption(const std::string &argv, std::string &option)
{
    size_t i;
    option.clear();
    for(i = 0; argv[i] == '-'; i++);
    option.assign(argv.cbegin() + i, argv.cend());
}

bool cmdline::find(const std::string &option)
{
    const auto it = cmdline_map.find(option);
    if(it == cmdline_map.cend())
        return false;
    return true;
}

bool cmdline::find(const std::string &option, std::string &argument)
{
    const auto it = cmdline_map.find(option);
    if(it == cmdline_map.cend())
        return false;
    argument = it->second;
    return true;
}

void cmdline::push(const std::string &option)
{
    cmdline_map[option] = std::string();
}

void cmdline::push(const std::string &option, const std::string &argument)
{
    cmdline_map[option] = argument;
}

void cmdline::remove(const std::string &option)
{
    cmdline_map.erase(option);
}
