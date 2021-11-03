/*
 * spawn_entity.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/entity_types.hpp>

namespace protocol::packets
{
// NOTENOTE: usually when this packet occurs, a pack of
// UpdateWhatever packets occur after that because SpawnEntity
// requires clients only spawn an entity and attach certain components
// to it but doesn't actually store any data related to these packets.
// So to spawn, lets say a generic creature we send SpawnEntity with an
// appropriate entity ID and type, and then we send UpdateCreature for it.
struct SpawnEntity final : public ServerPacket<0x008> {
    uint32_t entity_id;
    EntityType type;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(entity_id);
        s.value4b(type);
    }
};
} // namespace protocol::packets
