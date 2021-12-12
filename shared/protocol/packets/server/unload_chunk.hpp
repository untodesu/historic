/*
 * unload_chunk.hpp
 * Author: Kirill GPRB
 * Created: Sat Dec 11 2021 19:25:28
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/world.hpp>

namespace protocol::packets
{
struct UnloadChunk final : public ServerPacket<0x00B> {
    chunkpos_t::value_type position[3];

    template<typename S>
    inline void serialize(S &s)
    {
        s.container4b(position);
    }
};
} // namespace protocol::packets
