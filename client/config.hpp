/*
 * config.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 16:03:08
 */
#pragma once
#include <common/math/types.hpp>
#include <shared/config.hpp>

class ClientConfig final : public BaseConfig<ClientConfig> {
public:
    void implPostRead();
    void implPreWrite();

public:
    struct {
        bool enable;
        std::string host;
        uint16_t port;
    } autoconnect;
    struct {
        float fov, z_far;
        int shadowmapres;
        bool draw_shadows;
    } render;
    struct {
        int width, height;
        bool vsync, fullscreen;
    } window;
};
