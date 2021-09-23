/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/world.hpp>

struct ChunkComponent final {
    chunkpos_t position;
    ChunkComponent(const chunkpos_t &position);
};

inline ChunkComponent::ChunkComponent(const chunkpos_t &position)
    : position(position)
{
    
}
