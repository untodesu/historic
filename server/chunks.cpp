/*
 * chunks.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/filesystem.hpp>
#include <common/math/crc64.hpp>
#include <common/math/random.hpp>
#include <common/util/clock.hpp>
#include <server/chunks.hpp>
#include <server/globals.hpp>
#include <shared/components/chunk.hpp>
#include <spdlog/fmt/fmt.h>
#include <server/vgen.hpp>
#include <sstream>

void WorldConfig::implPostRead()
{
    base = toml["base"].value_or(0);
    height = toml["height"].value_or(2);
    generator.seed = math::crc64(toml["generator"]["seed"].value_or("0"));
    spdlog::info("seed = {}", generator.seed);
}

void WorldConfig::implPreWrite()
{
    // NOTE: we write only when world.toml doesn't exist
    // so we should be fine with random strings in preWrite()
    std::mt19937_64 rng = std::mt19937_64(util::seconds<uint64_t>(std::chrono::system_clock::now().time_since_epoch()));
    toml = toml::table {{
        { "base", base },
        { "height", height },
        { "generator", toml::table {{
            { "seed", math::randomString(rng, 16) }
        }}}
    }};
}

void ServerChunkManager::implOnClear()
{
    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, cp] : view.each())
        globals::registry.destroy(entity);
}

bool ServerChunkManager::implOnRemove(const chunkpos_t &cp, ServerChunk &data)
{
    if(--data.refcount > 1)
        return false;
    globals::registry.destroy(data.entity);
    return true;
}

ServerChunk ServerChunkManager::implOnCreate(const chunkpos_t &cp, voxel_set_flags_t)
{
    ServerChunk data;
    data.entity = globals::registry.create();
    globals::registry.emplace<ChunkComponent>(data.entity, ChunkComponent(cp));
    data.data.fill(NULL_VOXEL);
    data.refcount = 1;
    return std::move(data);
}

voxel_t ServerChunkManager::implGetVoxel(const ServerChunk &data, const localpos_t &lp) const
{
    return data.data[toVoxelIdx(lp)];
}

void ServerChunkManager::implSetVoxel(ServerChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags)
{
    // TODO: if the chunk is loaded, broadcast a packet
    data->data[toVoxelIdx(lp)] = voxel;
}

void ServerChunkManager::init()
{
    stdfs::create_directories(fs::getWritePath("world/chunks"));
    
    if(!config.read("world/world.toml")) {
        spdlog::warn("world.toml not found, creating a default one.");
        config.write("world/world.toml");
    }

    vgen.init(config);
}

void ServerChunkManager::shutdown()
{
    for(const auto it : chunks) {
        const voxel_t *data = it.second.data.data();
        const std::vector<uint8_t> buffer = std::vector<uint8_t>(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data + CHUNK_VOLUME));
        fs::writeBytes(fmt::format("world/chunks/c_{}_{}_{}", it.first.x, it.first.y, it.first.z), buffer);
    }
}

ServerChunk *ServerChunkManager::load(const chunkpos_t &cp)
{
    const auto it = chunks.find(cp);
    if(it != chunks.cend()) {
        it->second.refcount++;
        return &it->second;
    }

    voxel_array_t chunk;

    std::vector<uint8_t> buffer;
    if(fs::readBytes(fmt::format("world/chunks/c_{}_{}_{}", cp.x, cp.y, cp.z), buffer)) {
        const size_t max_sz = sizeof(voxel_t) * CHUNK_VOLUME;
        if(buffer.size() > max_sz)
            buffer.resize(max_sz);
        std::copy(buffer.cbegin(), buffer.cend(), chunk.begin());
    }
    else if(!vgen.generate(cp, chunk)) {
        spdlog::debug("null chunk at [{}, {}, {}]", cp.x, cp.y, cp.z);
        return nullptr;
    }

    ServerChunk *sc = create(cp, 0);
    sc->data = chunk;
    return sc;
}

void ServerChunkManager::free(const chunkpos_t &cp)
{
    const auto it = chunks.find(cp);
    if(it != chunks.cend()) {
        const voxel_t *data = it->second.data.data();
        const std::vector<uint8_t> buffer = std::vector<uint8_t>(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data + CHUNK_VOLUME));
        fs::writeBytes(fmt::format("world/chunks/c_{}_{}_{}", cp.x, cp.y, cp.z), buffer);
        remove(cp);
    }
}
