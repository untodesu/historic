/*
 * config.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 18:41:38
 */
#pragma once
#include <common/math/types.hpp>
#include <shared/config.hpp>

class ServerConfig final : public BaseConfig<ServerConfig> {
public:
    void implPostRead();
    void implPreWrite();

public:
    int32_t simulation_distance;
    struct {
        size_t maxplayers;
        uint16_t port;
    } net;
};
