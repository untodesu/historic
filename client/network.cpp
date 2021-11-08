/*
 * network.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <exception>
#include <client/chunks.hpp>
#include <client/globals.hpp>
#include <client/network.hpp>
#include <client/render/atlas.hpp>
#include <client/components/local_player.hpp>
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
#include <shared/script_engine.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <unordered_map>

struct NetIDComponent final { uint32_t id; };
static std::unordered_map<uint32_t, entt::entity> network_entities;
static std::unordered_map<uint32_t, ClientSession> sessions;

static void clearNetworkEntities()
{
    const auto view = globals::registry.view<NetIDComponent>();
    for(const auto [entity, netid] : view.each())
        globals::registry.destroy(entity);
    network_entities.clear();
}

static const std::unordered_map<uint16_t, void(*)(const std::vector<uint8_t> &)> packets = {
    {
        protocol::packets::LoginSuccess::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::LoginSuccess packet;
            protocol::deserialize(payload, packet);
            globals::session.id = packet.session_id;
            globals::session.state = SessionState::RECEIVING_GAMEDATA;
        }
    },
    {
        protocol::packets::VoxelDefEntry::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefEntry packet;
            protocol::deserialize(payload, packet);
            globals::voxels.build(packet.voxel).type(packet.type).submit();
        }
    },
    {
        protocol::packets::VoxelDefFace::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefFace packet;
            protocol::deserialize(payload, packet);
            globals::voxels.build(packet.voxel).face(packet.face).transparent(packet.flags & packet.TRANSPARENT_BIT).texture(packet.texture).endFace().submit();
        }
    },
    {
        protocol::packets::VoxelDefChecksum::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefChecksum packet;
            protocol::deserialize(payload, packet);

            uint64_t client_checksum = globals::voxels.getChecksum();
            if(client_checksum != packet.checksum) {
                // It's not that bad to disconnect and/or
                // try to receive the table again so we
                // are going to stay with a warning now.
                spdlog::warn("VoxelDef checksums differ! (client: {}, server: {})", client_checksum, packet.checksum);
            }

            globals::solid_textures.create(32, 32, MAX_VOXELS);
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                for(const auto face : it->second.faces)
                    globals::solid_textures.push(face.second.texture);
            }
            globals::solid_textures.submit();
        }
    },
    {
        protocol::packets::ChunkVoxels::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::ChunkVoxels packet;
            protocol::deserialize(payload, packet);
            globals::chunks.create(math::arrayToVec<chunkpos_t>(packet.position), CHUNK_CREATE_UPDATE_NEIGHBOURS)->data = packet.data;
        }
    },
    {
        protocol::packets::PlayerInfoEntry::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::PlayerInfoEntry packet;
            protocol::deserialize(payload, packet);
            network::createSession(packet.session_id);
        }
    },
    {
        protocol::packets::PlayerInfoUsername::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::PlayerInfoUsername packet;
            protocol::deserialize(payload, packet);

            if(ClientSession *session = network::findSession(packet.session_id)) {
                session->username = packet.username;
                if(session == &globals::session)
                    spdlog::info("Logged in as {}, session_id={}", session->username, session->id);
                return;
            }

            spdlog::warn("PlayerInfoUsername: unknown session_id: {}", packet.session_id);
        }
    },
    {
        protocol::packets::SpawnEntity::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::SpawnEntity packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = network::createEntity(packet.entity_id);
            switch(packet.type) {
                case EntityType::PLAYER:
                    globals::registry.emplace<CreatureComponent>(entity);
                    globals::registry.emplace<HeadComponent>(entity);
                    globals::registry.emplace<PlayerComponent>(entity);
                    break;
            }
        }
    },
    {
        protocol::packets::RemoveEntity::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::RemoveEntity packet;
            protocol::deserialize(payload, packet);
            network::removeEntity(packet.entity_id);
        }
    },
    {
        protocol::packets::SpawnPlayer::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::SpawnPlayer packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = network::findEntity(packet.entity_id);
            if(globals::registry.valid(entity)) {
                if(PlayerComponent *player = globals::registry.try_get<PlayerComponent>(entity)) {
                    player->session_id = packet.session_id;

                    if(ClientSession *session = network::findSession(player->session_id)) {
                        session->player_entity = entity;
                        session->player_entity_id = packet.entity_id;

                        if(player->session_id == globals::session.id) {
                            globals::registry.emplace<LocalPlayerComponent>(entity);
                            globals::session.state = SessionState::PLAYING;
                        }

                        return;
                    }
                }
            }

            // At this point player entities must be valid
            // and if they aren't it's just a red flag for us.
            network::disconnect("Protocol mishmash");
        }
    },
    {
        protocol::packets::ChatMessage::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::ChatMessage packet;
            protocol::deserialize(payload, packet);
            spdlog::info(packet.message);
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);

            spdlog::info("Disconnected: {}", packet.reason);

            enet_peer_disconnect(globals::session.peer, 0);

            globals::session.peer = nullptr;
            globals::session.id = 0;
            globals::session.state = SessionState::DISCONNECTED;
            globals::session.player_entity = entt::null;
            globals::session.player_entity_id = 0;

            globals::registry.clear();
        }
    },
    {
        protocol::packets::UpdateCreature::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::UpdateCreature packet;
            protocol::deserialize(payload, packet);
            entt::entity entity = network::findEntity(packet.entity_id);
            if(globals::registry.valid(entity)) {
                if(CreatureComponent *creature = globals::registry.try_get<CreatureComponent>(entity))
                    creature->position = math::arrayToVec<float3>(packet.position);
            }
        }
    },
    {
        protocol::packets::UpdateHead::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::UpdateHead packet;
            protocol::deserialize(payload, packet);
            entt::entity entity = network::findEntity(packet.entity_id);
            if(globals::registry.valid(entity)) {
                if(HeadComponent *head = globals::registry.try_get<HeadComponent>(entity))
                    head->angles = math::arrayToVec<float2>(packet.angles);
            }
        }
    }
};

namespace api
{
static duk_ret_t netConnect(duk_context *ctx)
{
    cl_network::connect(duk_safe_to_string(ctx, 0), duk_to_uint16(ctx, 1));
    return 0;
}

static duk_ret_t netDisconnect(duk_context *)
{
    cl_network::disconnect("Disconnected by User");
    return 0;
}
} // namespace api

void cl_network::preInit()
{
    globals::script.object("Net")
        .constant("DEFAULT_PORT", static_cast<int>(protocol::DEFAULT_PORT))
        .constant("LOCALHOST", "localhost")
        .function("connect", &api::netConnect, 2)
        .function("disconnect", &api::netDisconnect, 0)
        .submit();
}

void cl_network::init()
{
    globals::host = enet_host_create(nullptr, 1, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a client host object.");
        std::terminate();
    }

    globals::session.state = SessionState::DISCONNECTED;
}

void cl_network::shutdown()
{
    network::disconnect("Client shutting down");
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

bool cl_network::connect(const std::string &host, uint16_t port)
{
    network::disconnect("red vented");

    globals::registry.clear();

    ENetAddress address;
    address.port = port;
    if(enet_address_set_host(&address, host.c_str()) < 0) {
        spdlog::error("Unable to find {}:{}", host, port);
        return false;
    }

    globals::session.peer = enet_host_connect(globals::host, &address, 2, 0);
    if(!globals::session.peer) {
        spdlog::error("Unable to connect to {}:{}", host, port);
        return false;
    }

    ENetEvent event;
    while(enet_host_service(globals::host, &event, 5000) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::debug("Logging in...");

            protocol::packets::Handshake handshake = {};
            util::sendPacket(globals::session.peer, handshake, 0, 0);

            // TODO: change-able username
            protocol::packets::LoginStart login = {};
            login.username = "Kirill";
            util::sendPacket(globals::session.peer, login, 0, 0);

            globals::session.state = SessionState::LOGGING_IN;
            return true;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
            continue;
        }
    }

    enet_peer_reset(globals::session.peer);
    globals::session.peer = nullptr;

    spdlog::error("Unable to establish connection with {}:{}", host, port);
    return false;
}

void cl_network::disconnect(const std::string &reason)
{
    if(globals::session.peer) {
        protocol::packets::Disconnect packet;
        packet.reason = reason;
        util::sendPacket(globals::session.peer, packet, 0, 0);

        enet_peer_disconnect_later(globals::session.peer, 0);

        ENetEvent event;
        while(enet_host_service(globals::host, &event, 5000) > 0) {
            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected.");
                break;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                enet_packet_destroy(event.packet);
                continue;
            }
        }

        enet_peer_reset(globals::session.peer);

        globals::session.id = 0;
        globals::session.peer = nullptr;
        globals::session.player_entity = entt::null;
        globals::session.player_entity_id = 0;
        globals::session.state = SessionState::DISCONNECTED;

        clearNetworkEntities();
        
        sessions.clear();

        globals::chunks.clear();
        globals::voxels.clear();

        globals::registry.clear();
    }
}

void cl_network::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            const std::vector<uint8_t> packet = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t packet_id;
            std::vector<uint8_t> payload;
            if(!protocol::split(packet, packet_id, payload)) {
                spdlog::warn("Invalid packet format!");
                continue;
            }

            const auto it = packets.find(packet_id);
            if(it == packets.cend()) {
                spdlog::warn("Invalid packet 0x{:04X}", packet_id);
                continue;
            }

            it->second(payload);
        }
    }
}

entt::entity cl_network::createEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend()) {
        spdlog::warn("Network entity {} already exists!", network_id);
        return it->second;
    }

    entt::entity entity = globals::registry.create();
    globals::registry.emplace<NetIDComponent>(entity).id = network_id;
    return network_entities[network_id] = entity;
}

entt::entity cl_network::findEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend())
        return it->second;
    return entt::null;
}

void cl_network::removeEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend()) {
        globals::registry.destroy(it->second);
        network_entities.erase(it);
    }
}

ClientSession *cl_network::createSession(uint32_t session_id)
{
    if(session_id == globals::session.id)
        return &globals::session;
    
    const auto it = sessions.find(session_id);
    if(it != sessions.cend()) {
        spdlog::warn("Session {} already exists!", session_id);
        return &it->second;
    }

    ClientSession session = {};
    session.id = session_id;
    return &(sessions[session_id] = session);
}

ClientSession *cl_network::findSession(uint32_t session_id)
{
    if(session_id == globals::session.id)
        return &globals::session;
    const auto it = sessions.find(session_id);
    if(it != sessions.cend())
        return &it->second;
    return nullptr;
}

void cl_network::destroySession(ClientSession *session)
{
    const auto it = sessions.find(session->id);
    if(it != sessions.cend()) {
        if(globals::registry.valid(it->second.player_entity))
            network::removeEntity(it->second.player_entity_id);
        sessions.erase(it);
    }
}
