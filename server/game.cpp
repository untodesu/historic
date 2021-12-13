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

void sv_game::init()
{
    globals::chunks.init();
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
}

void sv_game::shutdown()
{
    globals::chunks.shutdown();
    globals::registry.clear();
}

void sv_game::update()
{

}
