#include "common/net/udp.hpp"

#include "common/context.hpp"
#include "common/event.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/net/net.hpp"
#include "enet/enet.h"
#include "proto/proto.pb.h"
#include "schema/proto/net_msg_dispatch.hpp"

NetAddress::NetAddress(uint64_t ip, uint32_t port) : m_host{ip}, m_port{port} {}

NetAddress::NetAddress(std::string_view ip, uint32_t port) : m_port{port} {
    ENetAddress address;
    if (enet_address_set_host(&address, ip.data()) != 0) {
        LOGE("IP can't solve: {}", ip);
    } else {
        m_host = address.host;
    }
}

std::string NetAddress::GetIP() const {
    ENetAddress address;
    address.host = m_host;
    address.port = m_port;
    char host[64] = {0};
    enet_address_get_host_ip(&address, host, sizeof(host));
    return host;
}

UDPPeer::UDPPeer(UDPHost* host, ENetPeer* peer) : m_host{host}, m_peer{peer} {
    if (peer) {
        m_id = peer->connectID;
    }
}

UDPPeer::UDPPeer(UDPHost* host, ENetPeer* peer, uint32_t id)
    : m_host{host}, m_peer{peer}, m_id{id} {}

void UDPPeer::Disconnect() {
    TL_RETURN_IF_NULL_WITH_LOG(m_host, LOGE, "host is null");
    TL_RETURN_IF_NULL_WITH_LOG(m_peer, LOGE, "peer is null");
    enet_peer_disconnect(m_peer, 0);
    m_id = 0;
}

UDPPeer::ID UDPPeer::GetID() const {
    return m_id;
}

std::string UDPPeer::GetIP() const {
    TL_RETURN_VALUE_IF_NULL(m_peer, {});
    char ip[64] = {0};
    enet_address_get_host_ip(&m_peer->address, ip, sizeof(ip));
    return ip;
}

uint32_t UDPPeer::GetHost() const {
    TL_RETURN_VALUE_IF_NULL(m_peer, 0);
    return m_peer->address.host;
}

uint16_t UDPPeer::GetPort() const {
    TL_RETURN_VALUE_IF_NULL(m_peer, 0);
    return m_peer->address.port;
}

bool UDPPeer::IsValid() const {
    return m_host && m_peer;
}

void UDPPeer::Reset() {
    TL_RETURN_IF_NULL(m_peer);
    if (m_host) {
        m_host->Flush();
    }
    enet_peer_reset(m_peer);
    m_peer = nullptr;
    m_host = nullptr;
}

UDPHost::UDPHost(const NetAddress* address, int peer_count) {
    ENetAddress enet_address;
    if (address) {
        enet_address.host = address->m_host;
        enet_address.port = address->m_port;
    }

    m_host = enet_host_create(address ? &enet_address : nullptr, peer_count, 0,
                              0, 0);
    if (address) {
        TL_RETURN_IF_NULL_WITH_LOG(m_host, LOGE,
                                   "create host on {}:{} failed, ",
                                   address->GetIP(), address->m_port);
    } else {
        TL_RETURN_IF_NULL_WITH_LOG(m_host, LOGE, "create host failed");
    }
}

UDPHost::~UDPHost() {
    TL_RETURN_IF_NULL(m_host);
    for (auto& [id, peer] : m_peers) {
        TL_CONTINUE_IF_FALSE(peer.IsValid());
        peer.Disconnect();
    }
    enet_host_flush(m_host);
    enet_host_destroy(m_host);
}

void UDPHost::Send(const UDPPeer* peer, const std::byte* buf, int len,
                   int channel_id, Flags<UDPPacketFlag> flags) const {
    TL_RETURN_IF_NULL_WITH_LOG(m_host, LOGW, "host is null");
    auto packet = enet_packet_create(buf, len, flags.Value());
    TL_RETURN_IF_NULL_WITH_LOG(packet, LOGE, "create packet failed");

    if (!peer) {
        enet_host_broadcast(m_host, channel_id, packet);
    } else {
        if (enet_peer_send(peer->m_peer, channel_id, packet) < 0) {
            enet_packet_destroy(packet);
            LOGE("send packet to peer {}:{} failed", peer->GetIP(),
                 peer->GetPort());
        }
    }
}

void UDPHost::Send(const UDPPeer* peer, const proto::NetMsg& net_msg,
                   int channel_id, Flags<UDPPacketFlag> flags) {
    m_data_cache.clear();
    m_data_cache.resize(net_msg.ByteSizeLong());

    if (net_msg.SerializeToArray(m_data_cache.data(), m_data_cache.size())) {
        Send(peer, m_data_cache.data(), m_data_cache.size(), channel_id, flags);
    }
}

UDPPeer UDPHost::Connect(const NetAddress& address) {
    TL_RETURN_DEFAULT_IF_NULL_WITH_LOG(m_host, LOGW, "host not create");

    ENetAddress enet_address;
    enet_address.host = address.m_host;
    enet_address.port = address.m_port;
    ENetPeer* peer = enet_host_connect(m_host, &enet_address, 2, 0);
    TL_RETURN_DEFAULT_IF_NULL_WITH_LOG(peer, LOGE,
                                       "connect to peer {}:{} failed",
                                       address.GetIP(), address.m_port);

    ENetEvent event;
    if (enet_host_service(m_host, &event, 5000) <= 0 ||
        event.type != ENET_EVENT_TYPE_CONNECT) {
        LOGE("connect to {}:{} timeout!", address.GetIP(), address.m_port);
        enet_peer_reset(peer);
        return {};
    }

    auto& event_system = COMMON_CONTEXT.m_event_system;
    auto [it, inserted] = m_peers.emplace(peer->connectID, UDPPeer{this, peer});
    TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(
        inserted, LOGE, "same peer ID! ID = {}, IP: {}:{}", peer->connectID,
        address.GetIP(), address.m_port);

    m_peer_ids[peer] = peer->connectID;

    proto::Connect connect;
    event_system->EnqueueEvent(NetMsg{it->second, connect});

    return it->second;
}

void UDPHost::Flush() const {
    TL_RETURN_IF_NULL(m_host);

    enet_host_flush(m_host);
}

void UDPHost::HandleIncomingNetPacket() {
    static ENetEvent event;
    auto& event_system = COMMON_CONTEXT.m_event_system;

    while (enet_host_service(m_host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            auto id = event.peer->connectID;
            auto [it, _] = m_peers.emplace(id, UDPPeer{this, event.peer});
            m_peer_ids[event.peer] = id;
            proto::Connect connect;
            event_system->EnqueueEvent(NetMsg{it->second, connect});
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            auto id = m_peer_ids[event.peer];
            m_peer_ids.erase(event.peer);
            m_peers.erase(id);

            proto::Disconnect disconnect;
            event_system->EnqueueEvent(NetMsg{
                UDPPeer{this, event.peer, id},
                disconnect
            });
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            auto data = event.packet->data;
            auto data_len = event.packet->dataLength;
            TL_CONTINUE_IF_FALSE(event.packet && data && data_len > 0);

            auto it = m_peers.find(event.peer->connectID);
            if (it != m_peers.end()) {
                NetMsgDispatch(it->second, data, data_len);
            } else {
                LOGE("receive packet from unknown peer");
            }
            enet_packet_destroy(event.packet);
        }
    }

    enet_host_flush(m_host);
}

bool UDPHost::IsValid() const {
    return m_host;
}

UDPPeer UDPHost::GetPeer(UDPPeer::ID id) const {
    const auto it = m_peers.find(id);
    if (it == m_peers.end()) {
        return {};
    }
    return it->second;
}

const std::unordered_map<UDPPeer::ID, UDPPeer>& UDPHost::GetAllPeers() const {
    return m_peers;
}

std::unordered_map<UDPPeer::ID, UDPPeer>& UDPHost::GetAllPeers() {
    return m_peers;
}

void UDPInit() {
    auto& event_system = COMMON_CONTEXT.m_event_system;

    NetMsgError error_msg;
    if (enet_initialize() != 0) {
        error_msg.m_error_msg = "enet init failed";
        event_system->EnqueueEvent(error_msg);
        LOGE(error_msg.m_error_msg);
    }
}

void UDPShutdown() {
    enet_deinitialize();
}
