#include "server/logic.hpp"

#include "common/log.hpp"
#include "common/logic.hpp"

#include "absl/strings/str_format.h"
#include "common/macros.hpp"
#include "enet/enet.h"
#include "proto/all_proto.pb.h"

void ServerLogic::OnInit() {
    ILogic::OnInit();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = PORT;

    initNetHost(&address, 32, 2, 0, 0);

    registerNetMsgDelegate(&ServerLogic::onConnectReceive);
    registerNetMsgDelegate(&ServerLogic::onDisconnectReceive);
    registerNetMsgDelegate(&ServerLogic::onTalkMsgReceive);
    registerNetMsgDelegate(&ServerLogic::onMoveReceive);
}

void ServerLogic::OnUpdate(TimeType elapse) {
    ILogic::OnUpdate(elapse);
}

void ServerLogic::OnQuit() {
    ILogic::OnQuit();
}

void ServerLogic::onConnectReceive(const NetMsg<proto::Connect>& msg) {
    ENetPeer* peer = msg.m_peer;
    char buf[1024] = {0};
    enet_address_get_host(&peer->address, buf, sizeof(buf) - 1);
    LOGI("connect client: {}", buf);

    {
        proto::NetMsg net_msg;
        auto* enter = net_msg.mutable_m_enter();
        enter->set_m_id(peer->connectID);
        Vec2 position = CalcPlankPos(m_side);
        auto* pos = enter->mutable_m_position();
        pos->set_m_x(position.x);
        pos->set_m_y(position.y);

        m_side *= -1;

        m_peers[peer] = Plank{peer, peer->connectID, position};

        std::vector<uint8_t> msg_buf(net_msg.ByteSizeLong());
        if (net_msg.SerializeToArray(msg_buf.data(), msg_buf.size())) {
            auto packet = enet_packet_create(msg_buf.data(), msg_buf.size(),
                                             ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(m_host, 0, packet);
        }
    }

    for (auto&& [id, other] : m_peers) {
        TL_CONTINUE_IF_TRUE(id->connectID == peer->connectID);

        proto::NetMsg net_msg;
        auto* enter = net_msg.mutable_m_enter();
        enter->set_m_id(other.m_peer->connectID);
        auto* pos = enter->mutable_m_position();
        pos->set_m_x(other.m_position.x);
        pos->set_m_y(other.m_position.y);

        std::vector<uint8_t> msg_buf(net_msg.ByteSizeLong());

        if (net_msg.SerializeToArray(msg_buf.data(), msg_buf.size())) {
            auto packet = enet_packet_create(msg_buf.data(), msg_buf.size(),
                                        ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(msg.m_peer, 0, packet);
        }
    }
}

void ServerLogic::onDisconnectReceive(const NetMsg<proto::Disconnect>& msg) {
    char host[1024] = {0};
    enet_address_get_host(&msg.m_peer->address, host, sizeof(host) - 1);
    LOGI("disconnect client: {}", host);

    auto it = m_peers.find(msg.m_peer);

    proto::NetMsg net_msg;
    auto* leave = net_msg.mutable_m_leave();
    if (it != m_peers.end()) {
        leave->set_m_id(it->second.m_id);
    }

    std::vector<uint8_t> msg_buf(net_msg.ByteSizeLong());
    if (net_msg.SerializeToArray(msg_buf.data(), msg_buf.size())) {
        auto packet = enet_packet_create(msg_buf.data(), msg_buf.size(), 0);
        if (packet) {
            enet_host_broadcast(m_host, 0, packet);
        }
    }

    m_peers.erase(msg.m_peer);
    m_side *= -1;
}

void ServerLogic::onTalkMsgReceive(const NetMsg<proto::TalkMsg>& msg) {
    LOGI("recieve msg: {}", msg->m_msg());

    proto::NetMsg net_msg;
    net_msg.mutable_m_talk_msg()->set_m_msg(msg->m_msg());

    std::vector<uint8_t> buf(net_msg.ByteSizeLong());
    if (net_msg.SerializeToArray(buf.data(), buf.size())) {
        ENetPacket* packet =
            enet_packet_create(buf.data(), buf.size(), ENET_PACKET_FLAG_RELIABLE);

        enet_host_broadcast(m_host, 0, packet);
    }
}

void ServerLogic::onMoveReceive(const NetMsg<proto::Move>& msg) {
    m_peers[msg.m_peer].m_position = Vec2{msg->m_position().m_x(), msg->m_position().m_y()};

    proto::NetMsg net_msg;
    *net_msg.mutable_m_move() = msg.Payload();

    std::vector<uint8_t> msg_buf(net_msg.ByteSizeLong());
    if (net_msg.SerializeToArray(msg_buf.data(), msg_buf.size())) {
        auto packet = enet_packet_create(msg_buf.data(), msg_buf.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(m_host, 0, packet);
    }
}
