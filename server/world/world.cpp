/*
 * world.cpp
 * Author: Kirill GPRB
 * Created: Mon Dec 13 2021 00:20:32
 */
#include <common/util/clock.hpp>
#include <server/world/config.hpp>
#include <server/world/vgen.hpp>
#include <server/world/world.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

static WorldConfig world_config;
static VGen vgen;

void world::init()
{
    if(!world_config.read("world/world.toml")) {
        world_config.generator.seed = util::seconds<uint64_t>(std::chrono::system_clock::now().time_since_epoch());
        stdfs::create_directories(fs::getWritePath("world/chunks"));
        world_config.write("world/world.toml");
    }

    vgen.init(world_config);
}

void world::shutdown()
{
    // nothing yet
}

bool world::load(const chunkpos_t &cp, voxel_array_t &chunk)
{
    std::vector<uint8_t> buffer;
    if(fs::readBytes(fmt::format("world/chunks/c_{}_{}_{}", cp.x, cp.y, cp.z), buffer)) {
        if(buffer.size() > CHUNK_VOLUME)
            buffer.resize(CHUNK_VOLUME);
        std::copy(buffer.cbegin(), buffer.cend(), chunk.begin());
        return true;
    }

    return false;
}

bool world::store(const chunkpos_t &cp, const voxel_array_t &chunk)
{
    const voxel_t *data = chunk.data();
    std::vector<uint8_t> buffer(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data + CHUNK_VOLUME));
    fs::writeBytes(fmt::format("world/chunks/c_{}_{}_{}", cp.x, cp.y, cp.z), buffer);
    return true;
}

bool world::generate(const chunkpos_t &cp, voxel_array_t &chunk)
{
    return vgen.generate(cp, chunk);
}
