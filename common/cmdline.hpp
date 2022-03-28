/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <string>

namespace cmdline
{
void initialize(int argc, char **argv);
void shutdown();
bool isOption(const std::string &argv);
void getOption(const std::string &argv, std::string &option);
bool find(const std::string &option);
bool find(const std::string &option, std::string &argument);
void push(const std::string &option);
void push(const std::string &option, const std::string &argument);
void remove(const std::string &option);
} // namespace cmdline
