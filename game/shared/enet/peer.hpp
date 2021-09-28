/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/enet/packet.hpp>

namespace enet
{
class PeerRef final {
public:
    PeerRef(const PeerRef &rhs) = delete;
    PeerRef(PeerRef &&rhs);
    PeerRef(ENetPeer *peer);
    PeerRef &operator=(const PeerRef &rhs) = delete;
    PeerRef &operator=(PeerRef &&rhs);
    void disconnect();
    void disconnectDefer();
    void reset();
    bool send(uint8_t channel, PacketRef &packet);
    ENetPeer *get() const;

private:
    ENetPeer *peer;
};
} // namespace enet

inline enet::PeerRef::PeerRef(enet::PeerRef &&rhs)
    : peer(rhs.peer)
{
    rhs.peer = nullptr;
}

inline enet::PeerRef::PeerRef(ENetPeer *peer)
    : peer(peer)
{

}

inline enet::PeerRef &enet::PeerRef::operator=(enet::PeerRef &&rhs)
{
    enet::PeerRef copy(std::move(rhs));
    std::swap(peer, copy.peer);
    return *this;
}

inline void enet::PeerRef::disconnect()
{
    if(!peer)
        return;
    enet_peer_disconnect(peer, 0);
}

inline void enet::PeerRef::disconnectDefer()
{
    if(!peer)
        return;
    enet_peer_disconnect_later(peer, 0);
}

inline void enet::PeerRef::reset()
{
    if(!peer)
        return;
    enet_peer_reset(peer);
}

inline bool enet::PeerRef::send(uint8_t channel, enet::PacketRef &packet)
{
    if(!peer)
        return false;
    return enet_peer_send(peer, channel, packet.invalidate());
}

inline ENetPeer *enet::PeerRef::get() const
{
    return peer;
}
