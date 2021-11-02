/*
 * debug_overlay.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/systems/chunk_mesher.hpp>
#include <client/systems/proj_view.hpp>
#include <client/debug_overlay.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <imgui.h>
#include <string>

constexpr static const ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar ;

void debug_overlay::draw()
{
    const float2 &size = screen::getSize();
    const float3 &pos = proj_view::position();
    const float2 &ang = glm::degrees(proj_view::angles());
    if(ImGui::Begin("debug_overlay", nullptr, WINDOW_FLAGS)) {
        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
        ImGui::Text("%.03f ms (%05.02f FPS)", globals::avg_frametime * 1000.0f, 1.0f / globals::avg_frametime);
        ImGui::Text("%zu vertices this frame", globals::vertices_drawn);
        ImGui::Text("chunk_mesher: %zu KiB allocated", chunk_mesher::memory() / 1024);
        ImGui::Text("pos: %.03f %.03f %.03f", pos.x, pos.y, pos.z);
        ImGui::Text("ang: %.03f %.03f", ang.x, ang.y);
        ImGui::End();
    }
}
