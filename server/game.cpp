/*
 * game.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <ctime>
#include <server/chunks.hpp>
#include <server/game.hpp>
#include <server/globals.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <glm/gtc/noise.hpp>
#include <spdlog/spdlog.h>
#include <random>
#include <unordered_map>

static inline float octanoise(const float3 &v, unsigned int oct)
{
    float result = 1.0;
    for(unsigned int i = 1; i <= oct; i++)
        result += glm::simplex(v * static_cast<float>(i));
    return result / static_cast<float>(oct);
}

static void generate(uint64_t seed = 0)
{
    constexpr const int64_t START = -128;
    constexpr const int64_t END = 128;

    std::mt19937_64 mtgen = std::mt19937_64(seed);
    const float seed_f = std::uniform_real_distribution<float>()(mtgen);
    for(int64_t vx = START; vx < END; vx++) {
        for(int64_t vz = START; vz < END; vz++) {
            const float3 vxz = float3(vx, vz, seed_f * 5120.0f);
            const float solidity = octanoise(vxz / 160.0f, 3);
            const float hmod = octanoise((vxz + 1.0f) / 160.0f, 8);
            if(solidity >= 0.2f) {
                int64_t h1 = ((solidity - 0.2f) * 32.0f);
                int64_t h2 = (hmod * 16.0f);
                for(int64_t vy = 1; vy < h1; vy++)
                    globals::chunks.set(voxelpos_t(vx, -vy, vz), 0x01, VOXEL_SET_FORCE);
                for(int64_t vy = 0; h1 && vy < h2; vy++)
                    globals::chunks.set(voxelpos_t(vx, vy, vz), (vy == h2 - 1) ? 0x03 : 0x02, VOXEL_SET_FORCE);
            }
        }
    }
}

void sv_game::init()
{

}

void sv_game::postInit()
{
    // Stone
    globals::voxels.build(0x01)
        .type(VOXEL_SOLID)
        .face(VOXEL_FACE_LF)
            .transparent(false)
            .texture("textures/stone.png")
            .endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_RT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_FT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_BK).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_UP).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_DN).endFace()
        .submit();

    // Dirt
    globals::voxels.build(0x02)
        .type(VOXEL_SOLID)
        .face(VOXEL_FACE_LF)
            .transparent(false)
            .texture("textures/dirt.png")
            .endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_RT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_FT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_BK).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_UP).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_DN).endFace()
        .submit();

    // Grass
    globals::voxels.build(0x03)
        .type(VOXEL_SOLID)
        .face(VOXEL_FACE_LF)
            .transparent(false)
            .texture("textures/grass_side.png")
            .endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_RT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_FT).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_BK).endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_UP)
            .transparent(false)
            .texture("textures/grass.png")
            .endFace()
        .face(VOXEL_FACE_LF, VOXEL_FACE_DN)
            .transparent(false)
            .texture("textures/dirt.png")
            .endFace()
        .submit();

    uint64_t seed = static_cast<uint64_t>(std::time(nullptr));
    spdlog::info("Generating ({})...", seed);
    generate(seed);
    spdlog::info("Generating done");
}

void sv_game::shutdown()
{
    globals::registry.clear();
}

void sv_game::update()
{

}
