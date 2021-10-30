/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
                    globals::chunks.set(voxelpos_t(vx, -vy, vz), 0x01, true);
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
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/stone.png" });
        globals::voxels.set(0x01, vinfo);
    }

    // Dirt
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/dirt.png" });
        globals::voxels.set(0x02, vinfo);
    }

    // Grass
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/grass.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/dirt.png" });
        globals::voxels.set(0x03, vinfo);
    }

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
