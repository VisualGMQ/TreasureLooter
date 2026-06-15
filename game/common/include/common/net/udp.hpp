#pragma once
#include "common/flag.hpp"
#include "proto/all_proto.pb.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetAddress ENetAddress;

class NetAddress {
public:
    NetAddress() = default;
    NetAddress(uint64_t ip, uint32_t port);
    NetAddress(std::string_view ip, uint32_t port);

    [[nodiscard]] std::string GetIP() const;

    uint64_t m_host{};
    uint32_t m_port{};
};

class UDPHost;

class UDPPeer {
public:
    friend class UDPHost;

    using ID = uint32_t;

    static constexpr ID InvalidID = 0;

    UDPPeer() = default;
    UDPPeer(UDPHost*, ENetPeer*);
    UDPPeer(UDPHost*, ENetPeer*, uint32_t id);

    void Disconnect();
    [[nodiscard]] ID GetID() const;
    [[nodiscard]] std::string GetIP() const;
    [[nodiscard]] uint32_t GetHost() const;
    [[nodiscard]] uint16_t GetPort() const;

    [[nodiscard]] bool IsValid() const;
    void Reset();

private:
    UDPHost* m_host{};
    ENetPeer* m_peer{};
    uint32_t m_id{};
};

// copied from enet directly
enum class UDPPacketFlag {
    Reliable = 0x01,
    Unsequenced = 0x02,
    // NoAllocate = 0x03, // not support currently
    UnreliableFragment = 0x04,
};

class UDPHost {
public:
    explicit UDPHost(const NetAddress*, int peer_count = 10);
    UDPHost(const UDPHost&) = delete;
    UDPHost& operator=(const UDPHost&) = delete;
    ~UDPHost();

    void Send(const UDPPeer*, const std::byte* buf, int len, int channel_id,
              Flags<UDPPacketFlag> = UDPPacketFlag::Reliable) const;

    void Send(const UDPPeer* peer, const proto::NetMsg& net_msg, int channel_id,
              Flags<UDPPacketFlag> = UDPPacketFlag::Reliable);

    UDPPeer Connect(const NetAddress&);

    // flush udp packet to network
    void Flush() const;

    void HandleIncomingNetPacket();

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] UDPPeer GetPeer(UDPPeer::ID) const;

    const std::unordered_map<UDPPeer::ID, UDPPeer>& GetAllPeers() const;
    std::unordered_map<UDPPeer::ID, UDPPeer>& GetAllPeers();

private:
    ENetHost* m_host{};
    std::unordered_map<UDPPeer::ID, UDPPeer> m_peers;
    std::vector<std::byte> m_data_cache;

    // NOTE: we need this because on Disconnect event, ENetPeer.connectID always
    // 0
    std::unordered_map<ENetPeer*, UDPPeer::ID> m_peer_ids;
};

void UDPInit();
void UDPShutdown();