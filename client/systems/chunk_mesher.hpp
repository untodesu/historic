/*
 * chunk_mesher.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <stddef.h>

namespace chunk_mesher
{
void shutdown();
void update();
size_t memory();
} // namespace chunk_mesher
