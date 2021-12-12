/*
 * config.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 23:54:09
 */
#pragma once
#include <shared/config.hpp>

class WorldConfig final : public BaseConfig<WorldConfig> {
public:
    void implPostRead();
    void implPreWrite();

public:
    struct {
        int32_t height_b;
        int32_t height_e;
        int32_t edge;
    } world;
    struct {
        uint64_t seed;
    } generator;
};
