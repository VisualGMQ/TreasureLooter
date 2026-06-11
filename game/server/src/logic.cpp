#include "server/logic.hpp"

#include "common/log.hpp"
#include "common/logic.hpp"
#include "common/net/udp.hpp"
#include "proto/all_proto.pb.h"
#include "server/context.hpp"

void ServerLogic::OnInit() {
    SERVER_CONTEXT.NetListen(NetAddress{0, PORT}, 32);

    registerNetMsgDelegate(&ServerLogic::onConnectReceive);
    registerNetMsgDelegate(&ServerLogic::onDisconnectReceive);
    registerNetMsgDelegate(&ServerLogic::onTalkMsgReceive);
    registerNetMsgDelegate(&ServerLogic::onMoveReceive);
}

void ServerLogic::OnUpdate(TimeType elapse) {}

void ServerLogic::OnQuit() {}

void ServerLogic::onConnectReceive(const NetMsg<proto::Connect>& msg) {
    auto& new_peer = msg.m_peer;
    auto id = new_peer.GetID();
    LOGI("connect client: {}:{}", new_peer.GetIP(), new_peer.GetPort());

    {
        proto::NetMsg net_msg;
        auto* enter = net_msg.mutable_m_enter();
        enter->set_m_id(id);
        Vec2 position = CalcPlankPos(m_side);
        auto* pos = enter->mutable_m_position();
        pos->set_m_x(position.x);
        pos->set_m_y(position.y);

        m_side *= -1;

        m_peers[id] = Plank{new_peer, id, position};

        SERVER_CONTEXT.m_net_host->Send(nullptr, net_msg, 0);
    }

    for (auto&& [other_id, other] : m_peers) {
        TL_CONTINUE_IF_TRUE(other_id == id);

        proto::NetMsg net_msg;
        auto* enter = net_msg.mutable_m_enter();
        enter->set_m_id(other.m_id);
        auto* pos = enter->mutable_m_position();
        pos->set_m_x(other.m_position.x);
        pos->set_m_y(other.m_position.y);

        SERVER_CONTEXT.m_net_host->Send( &new_peer, net_msg, 0);
    }
}

void ServerLogic::onDisconnectReceive(const NetMsg<proto::Disconnect>& msg) {
    auto& peer = msg.m_peer;
    TL_RETURN_IF_FALSE_WITH_LOG(peer.IsValid(), LOGE,
                                "disconnect with null peer");

    auto id = peer.GetID();
    LOGI("disconnect client: id={}, {}:{}", id, peer.GetIP(), peer.GetPort());

    auto it = m_peers.find(id);

    proto::NetMsg net_msg;
    auto* leave = net_msg.mutable_m_leave();
    if (it != m_peers.end()) {
        leave->set_m_id(it->second.m_id);
    }

    SERVER_CONTEXT.m_net_host->Send(nullptr, net_msg, 0);

    m_peers.erase(id);
    m_side *= -1;
    SERVER_CONTEXT.m_net_host->GetPeer(id).Reset();
}

void ServerLogic::onTalkMsgReceive(const NetMsg<proto::TalkMsg>& msg) {
    LOGI("recieve msg: {}", msg->m_msg());

    proto::NetMsg net_msg;
    net_msg.mutable_m_talk_msg()->set_m_msg(msg->m_msg());

    SERVER_CONTEXT.m_net_host->Send(nullptr, net_msg, 0);
}

void ServerLogic::onMoveReceive(const NetMsg<proto::Move>& msg) {
    auto id = msg.m_peer.GetID();
    auto& plank = m_peers[id];
    plank.m_position = Vec2{msg->m_position().m_x(), msg->m_position().m_y()};

    proto::NetMsg net_msg;
    *net_msg.mutable_m_move() = msg.Payload();

    SERVER_CONTEXT.m_net_host->Send(nullptr, net_msg, 0);
}
