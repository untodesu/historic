/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/camera.hpp>
#include <client/comp/local_player.hpp>
#include <client/globals.hpp>
#include <client/net_ecs.hpp>
#include <client/net_entities.hpp>
#include <common/math/math.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <shared/protocol/packets/server/attach_creature.hpp>
#include <shared/protocol/packets/server/attach_head.hpp>
#include <shared/protocol/packets/server/attach_player.hpp>
#include <shared/protocol/packets/server/create_entity.hpp>

static void onAttachCreature(const std::vector<uint8_t> &payload)
{
    protocol::packets::AttachCreature packet;
    protocol::deserialize(payload, packet);

    entt::entity entity = net_entities::find(packet.network_id);
    if(entity != entt::null) {
        CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(entity);
        creature.position = math::arrayToVec<float3>(packet.position);
        creature.yaw = packet.yaw;
    }
}

static void onAttachHead(const std::vector<uint8_t> &payload)
{
    protocol::packets::AttachHead packet;
    protocol::deserialize(payload, packet);

    entt::entity entity = net_entities::find(packet.network_id);
    if(entity != entt::null) {
        HeadComponent &head = globals::registry.emplace<HeadComponent>(entity);
        head.angles = math::arrayToVec<float3>(packet.angles);
        head.offset = math::arrayToVec<float3>(packet.offset);
    }
}

static void onAttachPlayer(const std::vector<uint8_t> &payload)
{
    protocol::packets::AttachPlayer packet;
    protocol::deserialize(payload, packet);

    entt::entity entity = net_entities::find(packet.network_id);
    if(entity != entt::null) {
        PlayerComponent &player = globals::registry.emplace<PlayerComponent>(entity);
        player.session_id = packet.session_id;

        if(packet.session_id == globals::session_id) {
            globals::registry.emplace<LocalPlayerComponent>(entity);
            globals::registry.emplace<ActiveCameraComponent>(entity);

            CameraComponent &camera = globals::registry.emplace<CameraComponent>(entity);
            camera.fov = glm::radians(90.0f);
            camera.z_near = 0.01f;
            camera.z_far = 1024.0f;

            globals::local_player_network_id = packet.network_id;
            globals::local_player = entity;
            globals::state = ClientState::PLAYING;
        }
    }
}

static void onCreateEntity(const std::vector<uint8_t> &payload)
{
    protocol::packets::CreateEntity packet;
    protocol::deserialize(payload, packet);
    net_entities::create(packet.network_id);
}

void net_ecs::init()
{
    globals::packet_handlers[protocol::packets::AttachCreature::id] = &onAttachCreature;
    globals::packet_handlers[protocol::packets::AttachHead::id] = &onAttachHead;
    globals::packet_handlers[protocol::packets::AttachPlayer::id] = &onAttachPlayer;
    globals::packet_handlers[protocol::packets::CreateEntity::id] = &onCreateEntity;
}
