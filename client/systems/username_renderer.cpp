/*
 * username_renderer.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/render/imgui_ext/window.hpp>
#include <client/systems/proj_view.hpp>
#include <client/systems/username_renderer.hpp>
#include <common/math/transform.hpp>
#include <common/util/format.hpp>
#include <shared/components/creature.hpp>
#include <shared/components/player.hpp>
#include <shared/session.hpp>
#include <client/globals.hpp>
#include <client/network.hpp>
#include <client/screen.hpp>
#include <spdlog/spdlog.h>

static imgui_ext::Window username_window("###username_kludge", ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

void username_renderer::drawImgui()
{
    username_window.open();
    if(username_window.begin()) {
        const float2 &ss = screen::getSize();
        const float3 &vp = proj_view::position();
        const float4x4 &wts = proj_view::matrix();

        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(ss.x, ss.y), ImGuiCond_Always);

        const auto group = globals::registry.group<CreatureComponent, PlayerComponent>();
        for(const auto [entity, creature, player] : group.each()) {
            const ClientSession *session = network::findSession(player.session_id);
            if(session && session != &globals::session) {
                // Practice shows that when a whatever point is
                // out of the clip distance, the Z value in the
                // normalized device space becomes greater than 1.0
                // Linear algebra sure is weird!
                const float3 ndc = math::worldToDevice(wts, creature.position);
                if(ndc.z < 1.0f) {
                    const float2 sp = math::deviceToScreen(ndc, ss);
                    const ImVec2 ts = ImGui::CalcTextSize(session->username.c_str());
                    ImGui::SetCursorPos(ImVec2(sp.x + ts.x * 0.5f, sp.y + ts.y * 0.5f));
                    ImGui::TextUnformatted(session->username.c_str());
                }
            }
        }

        username_window.end();
    }
}
