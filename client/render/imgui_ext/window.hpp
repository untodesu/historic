/*
 * window.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <imgui.h>
#include <string>

namespace imgui_ext
{
class Window final {
public:
    Window(const std::string &id, ImGuiWindowFlags flags = 0);
    Window(const std::string &id, const std::string &title, ImGuiWindowFlags flags = 0);

    void open();
    void close();
    void toggle();
    bool isOpen() const;

    bool begin();
    void end();

private:
    const std::string id;
    ImGuiWindowFlags flags;
    bool is_open;
};
} // namespace imgui_ext

inline imgui_ext::Window::Window(const std::string &id, ImGuiWindowFlags flags)
    : id(id), flags(flags), is_open(false)
{

}

inline imgui_ext::Window::Window(const std::string &id, const std::string &title, ImGuiWindowFlags flags)
    : id(id + "###" + title), flags(flags), is_open(false)
{

}

inline void imgui_ext::Window::open()
{
    is_open = true;
}

inline void imgui_ext::Window::close()
{
    is_open = false;
}

inline void imgui_ext::Window::toggle()
{
    is_open = !is_open;
}

inline bool imgui_ext::Window::isOpen() const
{
    return is_open;
}

inline bool imgui_ext::Window::begin()
{
    return is_open && ImGui::Begin(id.c_str(), &is_open, flags);
}

inline void imgui_ext::Window::end()
{
    ImGui::End();
}
