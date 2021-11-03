/*
 * game.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <exception>
#include <client/components/camera.hpp>
#include <client/components/local_player.hpp>
#include <client/systems/chunk_mesher.hpp>
#include <client/systems/player_look.hpp>
#include <client/systems/player_move.hpp>
#include <client/systems/proj_view.hpp>
#include <client/render/atlas.hpp>
#include <client/systems/chunk_renderer.hpp>
#include <client/deferred_pass.hpp>
#include <client/render/gbuffer.hpp>
#include <client/util/screenshots.hpp>
#include <client/chunks.hpp>
#include <client/debug_overlay.hpp>
#include <client/game.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/network.hpp>
#include <client/screen.hpp>
#include <client/shadow_manager.hpp>
#include <chrono>
#include <shared/components/creature.hpp>
#include <shared/components/head.hpp>
#include <shared/components/player.hpp>
#include <shared/protocol/packets/client/handshake.hpp>
#include <shared/protocol/packets/client/login_start.hpp>
#include <shared/protocol/packets/client/request_gamedata.hpp>
#include <shared/protocol/packets/server/login_success.hpp>
#include <shared/session.hpp>
#include <shared/util/clock.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

static constexpr const float TICK_DT = 1.0f / protocol::DEFAULT_TICKRATE;
static ChronoClock<std::chrono::system_clock> tick_clock;

void cl_game::init()
{
    chunk_renderer::init();
    deferred_pass::init();

    shadow_manager::init(8192, 8192);
    shadow_manager::setLightOrientation(floatquat(glm::radians(float3(45.0f, 0.0f, 45.0f))));
    shadow_manager::setPolygonOffset(float2(3.0f, 0.5f));
}

void cl_game::postInit()
{
    network::connect("192.168.1.215", protocol::DEFAULT_PORT);
    tick_clock.restart();
}

void cl_game::shutdown()
{
    globals::registry.clear();

    globals::solid_textures.destroy();

    globals::solid_gbuffer.shutdown();

    shadow_manager::shutdown();

    deferred_pass::shutdown();
    chunk_renderer::shutdown();
}

void cl_game::modeChange(int width, int height)
{
    globals::solid_gbuffer.init(width, height);
}

void cl_game::update()
{
    proj_view::update();

    bool is_playing = (globals::session.state == SessionState::PLAYING);
    if(is_playing) {
        // NOTENOTE: when the new chunks arrive (during the login stage
        // when clientside receives some important data like voxel info)
        // sometimes shit gets fucked and one side of a chunk becomes
        // visible. This is not a problem for now because these quads
        // are occluded by the actual geometry. Things will get hot only
        // when mesher will start to skip actual geometry leaving holes.
        chunk_mesher::update();

        player_look::update();
        player_move::update();

        if(util::seconds<float>(tick_clock.elapsed()) >= TICK_DT) {
            player_look::send();
            player_move::send();
            tick_clock.restart();
        }
    }

    input::enableCursor(!is_playing);
}

void cl_game::draw()
{
    // Draw things to GBuffers
    chunk_renderer::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int width, height;
    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    // Draw things to the main framebuffer
    deferred_pass::draw();
}

#include <imgui.h>
constexpr static const ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

void cl_game::drawImgui()
{
    debug_overlay::draw();
    
    // This is just a quick-and-dirty way
    // for me to see if the client recognizes
    // other players that join. Nothing more.
    if(ImGui::Begin("kludge", nullptr, WINDOW_FLAGS)) {
        const float2 &ss = screen::getSize();
        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(ss.x, ss.y), ImGuiCond_Always);
        const float4x4 &pv = proj_view::matrix();
        const auto group = globals::registry.group(entt::get<CreatureComponent, PlayerComponent>, entt::exclude<LocalPlayerComponent>);
        for(const auto [entity, creature, player] : group.each()) {
            float4 clip = pv * float4(creature.position, 1.0f);
            float3 ndc = float3(clip) / clip.w;
            float2 scr = (float2(ndc) + 1.0f) * 0.5f;
            scr.y = 1.0 - scr.y;
            scr *= ss;
            ImGui::SetCursorPos(ImVec2(scr.x, scr.y));
            ImGui::Text("ID_%u", player.session_id);
        }
        ImGui::End();
    }
}

void cl_game::postDraw()
{
    if(input::isKeyJustPressed(GLFW_KEY_F2))
        screenshots::jpeg(100);
}
