#include "common/logic.hpp"
#include "enet/enet.h"
#include "proto/all_proto.pb.h"
#include "proto/proto.pb.h"
#include "schema/proto/net_msg_dispatch.hpp"

Vec2 CalcPlankPos(int side) {
    auto logic_size = COMMON_CONTEXT.GetGameConfig().m_logic_size;

    auto half_win_size = Vec2{logic_size} * 0.5;
    if (side < 0) {
        return {-half_win_size.w + RectHalfSize.w, 0};
    }
    return {half_win_size.w - RectHalfSize.w, 0};
}

void ILogic::OnInit() {
    auto& event_system = COMMON_CONTEXT.m_event_system;

    NetMsgError error_msg;
    if (enet_initialize() != 0) {
        error_msg.m_error_msg = "enet init failed";
        event_system->EnqueueEvent(error_msg);
        LOGE(error_msg.m_error_msg);
    }
}

void ILogic::OnUpdate(TimeType) {
    ENetEvent event;
    auto& event_system = COMMON_CONTEXT.m_event_system;

    while (enet_host_service(m_host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            proto::Connect connect;
            event_system->EnqueueEvent(NetMsg{event.peer, connect});
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            proto::Disconnect disconnect;
            event_system->EnqueueEvent(NetMsg{event.peer, disconnect});
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {

            auto data = event.packet->data;
            auto data_len = event.packet->dataLength;
            TL_CONTINUE_IF_FALSE(event.packet && data && data_len > 0);

            NetMsgDispatch(event.peer, data, data_len);
        }
    }

    enet_host_flush(m_host);
}

void ILogic::OnQuit() {
    if (m_host) {
        enet_host_destroy(m_host);
    }
    enet_deinitialize();
}

void ILogic::initNetHost(ENetAddress* address, uint32_t peer_count,
                         uint32_t channel_limit, uint32_t incoming_bandwidth,
                         uint32_t outgoing_bandwidth) {
    auto& event_system = COMMON_CONTEXT.m_event_system;

    m_host = enet_host_create(address, peer_count, channel_limit,
                              incoming_bandwidth, outgoing_bandwidth);
    NetMsgError error_msg;
    if (!m_host) {
        error_msg.m_error_msg = "enet client create failed";
        event_system->EnqueueEvent(error_msg);
        LOGE(error_msg.m_error_msg);
    }
}