/*
 * network.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/util/format.hpp>
#include <enet/enet.h>
#include <exception>
#include <server/chunks.hpp>
#include <server/globals.hpp>
#include <server/network.hpp>
#include <shared/components/chunk.hpp>
#include <shared/components/creature.hpp>
#include <shared/components/head.hpp>
#include <shared/components/player.hpp>
#include <shared/protocol/packets/client/handshake.hpp>
#include <shared/protocol/packets/client/login_start.hpp>
#include <shared/protocol/packets/server/chunk_voxels.hpp>
#include <shared/protocol/packets/server/login_success.hpp>
#include <shared/protocol/packets/server/player_info_entry.hpp>
#include <shared/protocol/packets/server/player_info_username.hpp>
#include <shared/protocol/packets/server/remove_entity.hpp>
#include <shared/protocol/packets/server/spawn_entity.hpp>
#include <shared/protocol/packets/server/spawn_player.hpp>
#include <shared/protocol/packets/server/voxel_def_checksum.hpp>
#include <shared/protocol/packets/server/voxel_def_entry.hpp>
#include <shared/protocol/packets/server/voxel_def_face.hpp>
#include <shared/protocol/packets/shared/chat_message.hpp>
#include <shared/protocol/packets/shared/disconnect.hpp>
#include <shared/protocol/packets/shared/update_creature.hpp>
#include <shared/protocol/packets/shared/update_head.hpp>
#include <shared/protocol/protocol.hpp>
#include <shared/script/cvars.hpp>
#include <shared/script/cvar_numeric.hpp>
#include <shared/util/enet.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

static uint32_t session_id_base = 0;
static std::unordered_map<uint32_t, Session> sessions;

static const std::unordered_map<uint16_t, void(*)(const std::vector<uint8_t> &, Session *)> packet_handlers = {
    {
        protocol::packets::Handshake::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::Handshake packet;
            protocol::deserialize(payload, packet);

            if(packet.protocol_version != protocol::VERSION) {
                network::kick(session, util::format("Protocol versions differ (server: %hu, client: %hu)", protocol::VERSION, packet.protocol_version));
                return;
            }

            session->state = SessionState::LOGGING_IN;
        }
    },
    {
        protocol::packets::LoginStart::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::LoginStart packet;
            protocol::deserialize(payload, packet);

            session->state = SessionState::RECEIVING_GAMEDATA;
            session->username = packet.username;

            protocol::packets::LoginSuccess p = {};
            p.session_id = session->id;
            util::sendPacket(session->peer, p, 0, 0);

            //
            // This little maneuver will cost us 50 server ticks
            //

            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                protocol::packets::VoxelDefEntry entryp = {};
                entryp.voxel = it->first;
                entryp.type = it->second.type;
                util::sendPacket(session->peer, entryp, 0, 0);

                for(const auto face : it->second.faces) {
                    protocol::packets::VoxelDefFace facep = {};
                    facep.voxel = it->first;
                    facep.face = face.first;
                    facep.flags = 0;
                    if(face.second.transparent)
                        facep.flags |= facep.TRANSPARENT_BIT;
                    facep.texture = face.second.texture;
                    util::sendPacket(session->peer, facep, 0, 0);
                }
            }

            protocol::packets::VoxelDefChecksum checksump = {};
            checksump.checksum = globals::voxels.getChecksum();
            util::sendPacket(session->peer, checksump, 0, 0);

            const auto view = globals::registry.view<ChunkComponent>();
            for(const auto [entity, chunk] : view.each()) {
                protocol::packets::ChunkVoxels chunkp = {};
                math::vecToArray(chunk.position, chunkp.position);
                chunkp.data = globals::chunks.find(chunk.position)->data;
                util::sendPacket(session->peer, chunkp, 0, 0);
            }

            session->player_entity = globals::registry.create();
            globals::registry.emplace<CreatureComponent>(session->player_entity).position = FLOAT3_ZERO;
            globals::registry.emplace<HeadComponent>(session->player_entity).angles = FLOAT2_ZERO;

            for(auto it = sessions.cbegin(); it != sessions.cend(); it++) {
                protocol::packets::PlayerInfoEntry entryp = {};
                entryp.session_id = it->first;

                protocol::packets::PlayerInfoUsername namep = {};
                namep.session_id = it->first;
                namep.username = it->second.username;
                
                util::sendPacket(session->peer, entryp, 0, 0);
                util::sendPacket(session->peer, namep, 0, 0);
                
                if(it->first == session->id) {
                    util::broadcastPacket(globals::host, entryp, 0, 0, session->peer);
                    util::broadcastPacket(globals::host, namep, 0, 0, session->peer);
                }

                if(globals::registry.valid(it->second.player_entity)) {
                    protocol::packets::SpawnEntity spawnp = {};
                    spawnp.entity_id = static_cast<uint32_t>(it->second.player_entity);
                    spawnp.type = EntityType::PLAYER;

                    protocol::packets::UpdateCreature creaturep = {};
                    creaturep.entity_id = static_cast<uint32_t>(it->second.player_entity);
                    math::vecToArray(globals::registry.get<CreatureComponent>(it->second.player_entity).position, creaturep.position);

                    protocol::packets::UpdateHead headp = {};
                    headp.entity_id = static_cast<uint32_t>(it->second.player_entity);
                    math::vecToArray(globals::registry.get<HeadComponent>(it->second.player_entity).angles, headp.angles);

                    // Send stuff to the peer
                    util::sendPacket(session->peer, spawnp, 0, 0);
                    util::sendPacket(session->peer, creaturep, 0, 0);
                    util::sendPacket(session->peer, headp, 0, 0);

                    // Broadcast our spawn to other players
                    if(it->first == session->id) {
                        util::broadcastPacket(globals::host, spawnp, 0, 0, session->peer);
                        util::broadcastPacket(globals::host, creaturep, 0, 0, session->peer);
                        util::broadcastPacket(globals::host, headp, 0, 0, session->peer);
                    }

                    if(it->first != session->id) {
                        protocol::packets::SpawnPlayer playerp = {};
                        playerp.entity_id = static_cast<uint32_t>(it->second.player_entity);
                        playerp.session_id = it->first;
                        util::sendPacket(session->peer, playerp, 0, 0);
                    }
                }
            }

            // Notice that we send SpawnPlayer outside of the loop.
            // The reason being client-side state machine that changes
            // it's state to PLAYING at the exact moment a SpawnPlayer
            // packet with owning session_id is occured.
            protocol::packets::SpawnPlayer playerp = {};
            playerp.entity_id = static_cast<uint32_t>(session->player_entity);
            playerp.session_id = session->id;
            util::broadcastPacket(globals::host, playerp, 0, 0);

            session->state = SessionState::PLAYING;
        }
    },
    {
        protocol::packets::ChatMessage::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::ChatMessage packet;
            protocol::deserialize(payload, packet);
            util::broadcastPacket(globals::host, packet, 0, 0);
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);

            spdlog::info("{} ({}) has left the game ({})", session->username, session->id, packet.reason);

            if(globals::registry.valid(session->player_entity)) {
                protocol::packets::RemoveEntity removep = {};
                removep.entity_id = static_cast<uint32_t>(session->player_entity);
                util::broadcastPacket(globals::host, removep, 0, 0);
                globals::registry.destroy(session->player_entity);
            }

            enet_peer_disconnect(session->peer, 0);
        }
    },
    {
        protocol::packets::UpdateCreature::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::UpdateCreature packet;
            protocol::deserialize(payload, packet);
            util::broadcastPacket(globals::host, packet, 0, 0, session->peer);
        }
    },
    {
        protocol::packets::UpdateHead::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::UpdateHead packet;
            protocol::deserialize(payload, packet);
            util::broadcastPacket(globals::host, packet, 0, 0, session->peer);
        }
    }
};

static CVarNumeric<uint16_t> net_port("net.port", protocol::DEFAULT_PORT, FCVAR_ARCHIVE | FCVAR_READONLY);
static CVarNumeric<size_t> net_maxplayers("net.maxplayers", 16, FCVAR_ARCHIVE | FCVAR_READONLY);

void sv_network::preInit()
{
    cvars::insert(net_port);
    cvars::insert(net_maxplayers);
}

void sv_network::init()
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = net_port.getValue();

    globals::host = enet_host_create(&address, net_maxplayers.getValue(), 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a server host object.");
        std::terminate();
    }
}

void sv_network::shutdown()
{
    network::kickAll("Server shutting down.");
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

void sv_network::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            Session *session = network::createSession();
            session->peer = event.peer;
            session->state = SessionState::CONNECTED;
            event.peer->data = session;
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
            network::destroySession(reinterpret_cast<Session *>(event.peer->data));
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            Session *session = reinterpret_cast<Session *>(event.peer->data);

            const std::vector<uint8_t> pbuf = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t packet_id;
            std::vector<uint8_t> payload;
            if(!protocol::split(pbuf, packet_id, payload)) {
                spdlog::warn("Invalid packet format received from client {}", session->id);
                continue;
            }

            const auto it = packet_handlers.find(packet_id);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} from {}", packet_id, session->id);
                continue;
            }

            it->second(payload, session);
        }
    }
}

Session *sv_network::createSession()
{
    Session session = {};
    session.id = session_id_base++;
    return &(sessions[session.id] = session);
}

Session *sv_network::findSession(uint32_t session_id)
{
    const auto it = sessions.find(session_id);
    if(it != sessions.cend())
        return &it->second;
    return nullptr;
}

void sv_network::destroySession(Session *session)
{
    for(auto it = sessions.cbegin(); it != sessions.cend(); it++) {
        if(&it->second == session) {
            if(globals::registry.valid(it->second.player_entity))
                globals::registry.destroy(session->player_entity);
            sessions.erase(it);
            return;
        }
    }
}

void sv_network::kick(Session *session, const std::string &reason)
{
    if(session) {
        if(session->peer) {
            protocol::packets::Disconnect packet = {};
            packet.reason = reason;
            util::sendPacket(session->peer, packet, 0, 0);
            enet_peer_disconnect_later(session->peer, 0);
            enet_host_flush(globals::host);
        }

        sv_network::destroySession(session);
    }
}

void sv_network::kickAll(const std::string &reason)
{
    protocol::packets::Disconnect packet = {};
    packet.reason = reason;

    for(auto it = sessions.cbegin(); it != sessions.cend(); it++) {
        if(it->second.peer) {
            util::sendPacket(it->second.peer, packet, 0, 0);
            enet_peer_disconnect_later(it->second.peer, 0);
        }
    }

    sessions.clear();
    enet_host_flush(globals::host);
}
