/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/deferred_pass.hpp>
#include <client/game.hpp>
#include <client/gbuffer.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/player_look.hpp>
#include <client/player_move.hpp>
#include <client/screenshots.hpp>
#include <client/view.hpp>
#include <client/voxel_mesher.hpp>
#include <client/voxel_renderer.hpp>
#include <common/comp/creature_component.hpp>
#include <common/comp/player_component.hpp>
#include <common/voxels.hpp>

// test
#include <random>

void client_game::initialize()
{
    voxel_renderer::initialize();
    deferred_pass::initialize();

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
    std::uniform_int_distribution intdist(-64, 63);
    for(size_t i = 0; i < 65536; i++) {
        voxel_pos_t vpos = { intdist(rng), intdist(rng), intdist(rng) };
        globals::chunks.trySetVoxel(vpos, 0x01, VOXEL_SET_FORCE | VOXEL_SET_UPDATE_NEIGHBOURS);
    }

    // Create a player...
    entt::entity player = globals::registry.create();
    globals::registry.emplace<CreatureComponent>(player);
    globals::registry.emplace<PlayerComponent>(player);
}

void client_game::shutdown()
{
    voxel_mesher::shutdown();

    deferred_pass::shutdown();
    voxel_renderer::shutdown();
}

void client_game::mode(int width, int height)
{
    globals::main_gbuffer.create(width, height);
}

void client_game::update()
{
    player_look::update();
    player_move::update();

    view::update();

    voxel_mesher::update();
}

void client_game::renderWorld()
{
    voxel_renderer::renderWorld();
    deferred_pass::renderWorld();
}

void client_game::postRender()
{
    if(input::isKeyJustPressed(GLFW_KEY_F2))
        screenshots::take();
    if(input::isKeyJustPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(globals::window, GLFW_TRUE);
}
