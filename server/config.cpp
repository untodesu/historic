/*
 * config.cpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 18:47:06
 */
#include <common/math/math.hpp>
#include <shared/protocol/protocol.hpp>
#include <server/config.hpp>

void ServerConfig::implPostRead()
{
    net.maxplayers = static_cast<size_t>(toml["net"]["maxplayers"].value_or<unsigned int>(16));
    net.port = toml["net"]["port"].value_or(protocol::DEFAULT_PORT);
}

void ServerConfig::implPreWrite()
{
    toml = toml::table {{
        { "net", toml::table {{
            { "maxplayers", static_cast<unsigned int>(net.maxplayers) },
            { "port", net.port }
        }}}
    }};
}
