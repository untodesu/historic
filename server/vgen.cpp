/*
 * vgen.cpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:39:48
 */
#include <common/math/types.hpp>
#include <glm/gtc/noise.hpp>
#include <server/chunks.hpp>
#include <server/vgen.hpp>

static inline float octanoise(const double3 &v, unsigned int oct)
{
    double result = 1.0;
    for(unsigned int i = 1; i <= oct; i++)
        result += glm::perlin(v / static_cast<double>(i));
    return static_cast<float>(result / static_cast<double>(oct + 1));
}

void VGen::init(const WorldConfig &config)
{
    base = config.base;
    height = static_cast<float>((cheight = config.height) * CHUNK_SIZE);
    smooth = math::max(config.toml["generator"]["vgen_smooth"].value_or(64.0f), 1.0f);
    rng = std::mt19937_64(seed = config.generator.seed);
    fseed = std::uniform_real_distribution<float>()(rng);
}

bool VGen::generate(const chunkpos_t &cp, voxel_array_t &chunk)
{
    chunk.fill(NULL_VOXEL);

    // Don't generate past these values
    if(cp.y < base || cp.y > base + cheight)
        return false;
    bool dirty = false;

    for(int16_t x = 0; x < CHUNK_SIZE; x++) {
        for(int16_t z = 0; z < CHUNK_SIZE; z++) {
            const voxelpos_t vp = toVoxelPos(cp, localpos_t(x, 0, z));
            const double3 vxz = double3(vp.x / smooth, fseed, vp.z / smooth);
            const float hmod = octanoise(vxz, 2);
            const size_t h = static_cast<size_t>(hmod * (height - 1.0f));
            const int32_t ch = toChunkPos(voxelpos_t(0, h, 0)).y + base;

            size_t ht = 0;
            if(cp.y < ch)
                ht = CHUNK_SIZE;
            else if(cp.y == ch)
                ht = h % CHUNK_SIZE;

            for(size_t i = 0; i < ht; i++) {
                chunk[toVoxelIdx(localpos_t(x, i, z))] = 0x01;
                dirty = true;
            }
        }
    }

    return dirty;
}
