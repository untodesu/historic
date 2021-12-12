/*
 * config.cpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:55:28
 */
#include <server/world/config.hpp>

void WorldConfig::implPostRead()
{
    world.height_b = toml["world"]["height_b"].value_or(-16);
    world.height_e = toml["world"]["height_e"].value_or(0);
    world.edge = toml["world"]["edge"].value_or(16);
    generator.seed = toml["generator"]["seed"].value_or(0);
}

void WorldConfig::implPreWrite()
{
    toml = toml::table {{
        { "world", toml::table {{
            { "height_b", world.height_b },
            { "height_e", world.height_e },
            { "edge", world.edge }
        }}},
        { "generator", toml::table {{
            { "seed", generator.seed }
        }}}
    }};
}
