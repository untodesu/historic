/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/shared/protocol/packets/shared/disconnect.hpp>
#include <game/shared/util/enet.hpp>
#include <game/server/globals.hpp>
#include <game/server/session_manager.hpp>

static uint32_t session_id_base = 0;
static std::unordered_map<uint32_t, Session> sessions;

void session_manager::init()
{
    kickAll("Session manager has restarted.");
    session_id_base = 0;
    sessions.clear();
}

Session *session_manager::create()
{
    Session session = {};
    session.session_id = session_id_base++;
    return &(sessions[session.session_id] = session);
}

Session *session_manager::find(uint32_t session_id)
{
    const auto it = sessions.find(session_id);
    if(it != sessions.cend())
        return &it->second;
    return nullptr;
}

void session_manager::destroy(Session *session)
{
    for(auto it = sessions.cbegin(); it != sessions.cend(); it++) {
        if(&it->second == session) {
            if(session->player != entt::null)
                globals::registry.destroy(session->player);
            sessions.erase(it);
            return;
        }
    }
}

void session_manager::kick(Session *session, const std::string &reason)
{
    if(session) {
        if(session->peer) {
            protocol::packets::Disconnect packet = {};
            packet.reason = reason;
            util::sendPacket(session->peer, packet, 0, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_disconnect_later(session->peer, 0);
            enet_host_flush(globals::host);
        }

        destroy(session);
    }
}

void session_manager::kickAll(const std::string &reason)
{
    protocol::packets::Disconnect packet = {};
    packet.reason = reason;

    for(auto it = sessions.cbegin(); it != sessions.cend(); it++) {
        if(it->second.peer) {
            util::sendPacket(it->second.peer, packet, 0, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_disconnect_later(it->second.peer, 0);
        }
    }

    sessions.clear();
    enet_host_flush(globals::host);
}
