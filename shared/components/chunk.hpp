/*
 * chunk.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/world.hpp>

struct ChunkComponent final {
    chunkpos_t position { 0, 0, 0 };
    ChunkComponent() = default;
    ChunkComponent(const chunkpos_t &cp)
        : position(cp)
    {

    }
};
