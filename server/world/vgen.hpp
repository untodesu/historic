/*
 * vgen.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:33:46
 */
#pragma once
#include <shared/chunks.hpp>
#include <server/world/config.hpp>
#include <random>

class VGen final {
public:
    void init(const WorldConfig &config);
    bool generate(const chunkpos_t &cp, voxel_array_t &chunk);

private:
    int32_t begin, end, edge;
    uint64_t seed;
    float world_height;
    float terrain_seed;
    float height_power;
    std::mt19937_64 mtgen;
};
