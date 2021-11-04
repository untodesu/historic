/*
 * console.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/console.hpp>
#include <client/input.hpp>
#include <client/screen.hpp>
#include <client/render/imgui_ext/window.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <common/util/spdlog.hpp>
#include <deque>

static constexpr const size_t MAX_CONSOLE_SIZE = 128;
static std::deque<std::string> console_deque;
static imgui_ext::Window console_window("Console", "console", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

class ConsoleSink final : public spdlog::sinks::base_sink<std::mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t buffer;
        formatter_->format(msg, buffer);
        console_deque.push_back(fmt::to_string(buffer));
    }

    void flush_()
    {

    }
};

void console::init()
{
    spdlog::default_logger_raw()->sinks().push_back(util::createSink<ConsoleSink>());
}

void console::update()
{
    if(input::isKeyJustPressed(GLFW_KEY_GRAVE_ACCENT))
        console_window.toggle();
    while(console_deque.size() > MAX_CONSOLE_SIZE)
        console_deque.pop_front();
}

void console::drawImgui()
{
    if(console_window.begin()) {
        const float2 &ss = screen::getSize();
        
        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(ss.x, ss.y), ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));
        for(auto it = console_deque.cbegin(); it != console_deque.cend(); it++)
            ImGui::TextUnformatted(it->c_str());
        ImGui::PopStyleVar();
        
        console_window.end();
    }
}
