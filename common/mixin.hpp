/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace mixin
{
/**
 * Makes a class non-copyable.
 */
class NoCopy {
public:
    NoCopy() = default;
    NoCopy(const NoCopy &rhs) = delete;
    NoCopy &operator=(const NoCopy &rhs) = delete;
};

/**
 * Makes a class non-movable.
 */
class NoMove {
public:
    NoMove() = default;
    NoMove(NoMove &&rhs) = delete;
    NoMove &operator=(NoMove &&rhs) = delete;
};
} // namespace mixin
