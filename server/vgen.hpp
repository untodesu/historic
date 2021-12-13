/*
 * vgen.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:33:46
 */
#pragma once
#include <shared/chunks.hpp>
#include <random>

class WorldConfig;
class VGen final {
public:
    void init(const WorldConfig &config);
    bool generate(const chunkpos_t &cp, voxel_array_t &chunk);

private:
    int32_t base, cheight;
    float height, smooth;
    std::mt19937_64 rng;
    uint64_t seed;
    float fseed;
};
