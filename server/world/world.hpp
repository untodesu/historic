/*
 * world.hpp
 * Author: Kirill GPRB
 * Created: Mon Dec 13 2021 00:16:40
 */
#pragma once
#include <common/filesystem.hpp>
#include <shared/world.hpp>

namespace world
{
void init();
void shutdown();
bool load(const chunkpos_t &cp, voxel_array_t &chunk);
bool store(const chunkpos_t &cp, const voxel_array_t &chunk);
bool generate(const chunkpos_t &cp, voxel_array_t &chunk);
} // namespace world
