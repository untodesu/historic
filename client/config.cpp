/*
 * config.cpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 16:04:31
 */
#include <common/math/math.hpp>
#include <client/config.hpp>
#include <shared/protocol/protocol.hpp>

void ClientConfig::implPostRead()
{
    autoconnect.enable = toml["autoconnect"]["enable"].value_or(false);
    autoconnect.host = toml["autoconnect"]["host"].value_or("localhost");
    autoconnect.port = toml["autoconnect"]["host"].value_or(protocol::DEFAULT_PORT);
    render.fov = math::clamp(toml["render"]["fov"].value_or(90.0f), 50.0f, 120.0f);
    render.z_far = math::clamp(toml["render"]["z_far"].value_or(512.0f), 32.0f, 5120.0f);
    render.shadowmapres = math::pow2(math::clamp(toml["render"]["shadowmapres"].value_or(2048), 512, 8192));
    render.draw_shadows = toml["render"]["draw_shadows"].value_or(true);
    window.width = math::max(0, toml["window"]["width"].value_or(1152));
    window.height = math::max(0, toml["window"]["height"].value_or(648));
    window.vsync = toml["window"]["vsync"].value_or(true);
    window.fullscreen = toml["window"]["fullscreen"].value_or(false);
}

void ClientConfig::implPreWrite()
{
    toml = toml::table {{
        { "autoconnect", toml::table {{
            { "enable", autoconnect.enable },
            { "host", autoconnect.host },
            { "port", autoconnect.port }
        }}},
        { "render", toml::table {{
            { "fov", render.fov },
            { "z_far", render.z_far },
            { "shadowmapres", render.shadowmapres },
            { "draw_shadows", render.draw_shadows },
        }}},
        { "window", toml::table {{
            { "width", window.width },
            { "height", window.height },
            { "vsync", window.vsync },
            { "fullscreen", window.fullscreen },
        }}}
    }};
}
