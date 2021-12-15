/*
 * chunk_mesher.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <stddef.h>

struct ChunkMesherStats final {
    size_t thread_queue_size;
    size_t worker_quota;
    size_t worker_count;
};

namespace chunk_mesher
{
void shutdown();
void update();
void stats(ChunkMesherStats &s);
} // namespace chunk_mesher
