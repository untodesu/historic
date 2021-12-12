/*
 * vgen.cpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:39:48
 */
#include <common/math/types.hpp>
#include <glm/gtc/noise.hpp>
#include <server/world/vgen.hpp>

static inline float octanoise(const double3 &v, unsigned int oct)
{
    double result = 1.0;
    for(unsigned int i = 1; i <= oct; i++)
        result += glm::simplex(v * static_cast<double>(i));
    return static_cast<float>(result / static_cast<double>(oct));
}

void VGen::init(const WorldConfig &config)
{
    begin = config.world.height_b;
    end = config.world.height_e;
    edge = config.world.edge;
    world_height = static_cast<float>(end - begin);
    mtgen.seed(seed = config.generator.seed);
    terrain_seed = std::uniform_real_distribution<float>()(mtgen) * config.toml["generator"]["vgen_tspow"].value_or<float>(5120.0f);
    height_power = config.toml["generator"]["vgen_hpow"].value_or(0.5f);
}

bool VGen::generate(const chunkpos_t &cp, voxel_array_t &chunk)
{
    chunk.fill(NULL_VOXEL);

    // Don't generate past these values
    if(cp.y < begin || cp.y > end || glm::abs(cp.x) > edge || glm::abs(cp.z) > edge)
        return false;
    bool dirty = false;

    for(int16_t x = 0; x < CHUNK_SIZE; x++) {
        for(int16_t z = 0; z < CHUNK_SIZE; z++) {
            const voxelpos_t vp = toVoxelPos(cp, localpos_t(x, 0, z));
            const double3 vxz = double3(vp.x, terrain_seed, z);
            const float hmod = octanoise(vxz, 4);
            const size_t h = math::min(static_cast<size_t>(hmod * world_height * height_power), CHUNK_SIZE);
            for(size_t i = 0; i < h; i++) {
                chunk[toVoxelIdx(localpos_t(x, i, z))] = 0x01;
                dirty = true;
            }
        }
    }

    return dirty;
}
