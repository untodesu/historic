/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/types.hpp>

namespace protocol
{
constexpr static const uint16_t VERSION = 0x0001;
constexpr static const uint16_t DEFAULT_PORT = 43103;
constexpr static const float SERVER_TICKRATE = 30.0f;
constexpr static const size_t USERNAME_SIZE = 39;
} // namespace protocol
