/*
 * console.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/console.hpp>
#include <client/screen.hpp>
#include <client/fontlib.hpp>
#include <client/globals.hpp>
#include <client/input.hpp>
#include <client/render/imgui_ext/window.hpp>
#include <shared/script/script_engine.hpp>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <common/util/spdlog.hpp>
#include <deque>
#include <sstream>

static constexpr const size_t MAX_CONSOLE_SIZE = 128;
static std::deque<std::string> console_deque;
static imgui_ext::Window console_window("Console", "console", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
static bool scroll_to_bottom = true;
static std::string input_buffer;
static ImFont *font = nullptr;

class ConsoleSink final : public spdlog::sinks::base_sink<std::mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t buffer;
        formatter_->format(msg, buffer);
        console_deque.push_back(fmt::to_string(buffer));
        scroll_to_bottom = true;
    }

    void flush_()
    {

    }
};

namespace api
{
static duk_ret_t consoleClear(duk_context *)
{
    console_deque.clear();
    return 0;
}

static duk_ret_t consolePrint(duk_context *ctx)
{
    const duk_idx_t argc = duk_get_top(ctx);
    if(argc) {
        std::stringstream ss;
        for(duk_idx_t i = 0; i < argc; i++)
            ss << duk_safe_to_string(ctx, i);
        console_deque.push_back(ss.str());
    }

    return 0;
}
} // namespace api

void console::preInit()
{
    globals::script.build("Console")
        .function("clear", &api::consoleClear, 0)
        .function("print", &api::consolePrint, DUK_VARARGS)
        .submit();
    spdlog::default_logger_raw()->sinks().push_back(util::createSink<ConsoleSink>());
}

void console::init()
{
    font = nullptr;
    input_buffer.clear();
}

void console::update()
{
    if(input::isKeyPressed(GLFW_KEY_F3) && input::isKeyJustPressed(GLFW_KEY_C))
        console_window.toggle();
    while(console_deque.size() > MAX_CONSOLE_SIZE)
        console_deque.pop_front();
    globals::ui_grabs_input = globals::ui_grabs_input || console_window.isOpen();
}

void console::drawImgui()
{
    ImGui::PushFont(font ? font : (font = fontlib::find("Console")));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if(console_window.begin()) {
        const float2 &ss = screen::getSize();

        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetWindowSize(ImVec2(ss.x, ss.y), ImGuiCond_Always);

        ImGui::BeginChild("###scrolling_region", ImVec2(0.0f, -1.0f * (ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeight())));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));
        
        for(auto it = console_deque.cbegin(); it != console_deque.cend(); it++)
            ImGui::TextUnformatted(it->c_str());

        if(scroll_to_bottom) {
            ImGui::SetScrollHereY(1.0f);
            scroll_to_bottom = false;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::SetNextItemWidth(-1.0f * ImGui::GetStyle().ItemSpacing.x);
        if(ImGui::InputText("###input", &input_buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            globals::script.eval(input_buffer);
            input_buffer.clear();
        }

        ImGui::SetKeyboardFocusHere(-1);

        console_window.end();
    }

    ImGui::PopStyleVar();
    ImGui::PopFont();
}
