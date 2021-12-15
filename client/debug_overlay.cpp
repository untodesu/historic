/*
 * debug_overlay.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/math/types.hpp>
#include <client/systems/chunk_mesher.hpp>
#include <client/systems/proj_view.hpp>
#include <client/debug_overlay.hpp>
#include <client/globals.hpp>
#include <client/screen.hpp>
#include <client/render/imgui_ext/window.hpp>
#include <shared/world.hpp>

static imgui_ext::Window debug_window("###debug_overlay", ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

void debug_overlay::drawImgui()
{
    debug_window.open();
    if(debug_window.begin()) {
        const float2 &ss = screen::getSize();
        const float3 &pos = proj_view::position();
        const float2 ang = glm::degrees(proj_view::angles());
        const chunkpos_t &cp = toChunkPos(pos);

        ChunkMesherStats stats;
        chunk_mesher::stats(stats);

        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(ss.x, ss.y), ImGuiCond_Always);

        ImGui::Text("%.03f ms (%05.02f FPS)", globals::avg_frametime * 1000.0f, 1.0f / globals::avg_frametime);
        ImGui::Text("%zu vertices this frame", globals::vertices_drawn);
        ImGui::Text("mesher: T/Q/C: %zu/%zu/%zu", stats.thread_queue_size, stats.worker_quota, stats.worker_count);
        ImGui::Text("pos: %.03f %.03f %.03f", pos.x, pos.y, pos.z);
        ImGui::Text("cpos: %" PRId32 " %" PRId32 " %" PRId32, cp.x, cp.y, cp.z);
        ImGui::Text("ang: %.03f %.03f", ang.x, ang.y);

        debug_window.end();
    }
}
