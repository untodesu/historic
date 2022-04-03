/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/game.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/player_look.hpp>
#include <client/player_move.hpp>
#include <client/terrain_mesher.hpp>
#include <client/terrain_renderer.hpp>
#include <client/view.hpp>
#include <common/comp/creature_component.hpp>
#include <common/comp/player_component.hpp>
#include <common/voxels.hpp>

// test
#include <random>

void client_game::initialize()
{
    terrain_renderer::initialize();

}

void client_game::postInitialize()
{
    input::enableCursor(false);

    // Add a simple "stone" cube.
    globals::voxels.build(0x01)
        .type(VoxelType::STATIC_CUBE)
        .face(VoxelFace::LF)
            .texture("textures/cube1.png")
            .texture("textures/cube2.png")
            .texture("textures/cube3.png")
            .texture("textures/cube4.png")
            .texture("textures/cube5.png")
            .texture("textures/cube6.png")
            .texture("textures/cube7.png")
            .texture("textures/cube8.png")
            .endFace()
        .face(VoxelFace::LF, VoxelFace::RT).endFace()
        .face(VoxelFace::LF, VoxelFace::FT).endFace()
        .face(VoxelFace::LF, VoxelFace::BK).endFace()
        .face(VoxelFace::LF, VoxelFace::UP).endFace()
        .face(VoxelFace::LF, VoxelFace::DN).endFace()
        .submit();

    globals::terrain_atlas.create(16, 16, static_cast<int>(globals::voxels.approximateTextureAmount()));
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        for(const auto &face : it->second.faces) {
            for(const auto &texture : face.second.textures) {
                // FIXME: determine the voxel type here!!!!
                globals::terrain_atlas.push(texture);
            }
        }
    }
    globals::terrain_atlas.submit();

    // test
    std::mt19937_64 rng(1234);
    std::uniform_int_distribution intdist1(-128, 127);
    std::uniform_int_distribution intdist2(0, 63);
    for(size_t i = 0; i < 65536*16; i++) {
        voxel_pos_t vpos = { intdist1(rng), intdist2(rng), intdist1(rng) };
        globals::chunks.trySetVoxel(vpos, 0x01, VOXEL_SET_FORCE | VOXEL_SET_UPDATE_NEIGHBOURS);
    }
    for(voxel_idx_t i = 0; i < CHUNK_VOLUME; i++) {
        globals::chunks.trySetVoxel(world::getVoxelPosition({0,0,0}, world::getLocalPosition(i)), 0x01, VOXEL_SET_FORCE | VOXEL_SET_UPDATE_NEIGHBOURS);
    }

    // Create a player...
    entt::entity player = globals::registry.create();
    globals::registry.emplace<CreatureComponent>(player);
    globals::registry.emplace<PlayerComponent>(player);
}

void client_game::shutdown()
{
    terrain_mesher::shutdown();
    terrain_renderer::shutdown();
}

void client_game::mode(int width, int height)
{

}

void client_game::update()
{
    player_look::update();
    player_move::update();

    view::update();

    terrain_mesher::update();
}

void client_game::renderWorld()
{
    terrain_renderer::renderWorld();
}

void client_game::postRender()
{
    if(input::isKeyJustPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(globals::window, GLFW_TRUE);
}
