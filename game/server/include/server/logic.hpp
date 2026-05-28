#pragma once
#include "common/logic.hpp"
#include "common/timer.hpp"
#include "proto/proto.pb.h"
// #include "enet/enet.h"

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

struct Plank {
    ENetPeer* m_peer{};
    uint32_t m_id;
    Vec2 m_position;
};

// NOTE: temporary class for learning networking, we write network code here
// rather than put into luau script
class ServerLogic: public ILogic {
public:
    void OnInit() override;
    void OnUpdate(TimeType) override;
    void OnQuit() override;

private:
    std::unordered_map<ENetPeer*, Plank> m_peers;
    int m_side = -1; // -1 means left, 1 means right

    void onConnectReceive(const NetMsg<proto::Connect>&);
    void onDisconnectReceive(const NetMsg<proto::Disconnect>&);
    void onTalkMsgReceive(const NetMsg<proto::TalkMsg>&);
    void onMoveReceive(const NetMsg<proto::Move>&);

    template <typename T, typename Class>
    void registerNetMsgDelegate(void (Class::*f)(const T&)) {
        ILogic::registerNetMsgDelegate<typename T::message_type>(
            [this, f](EventListenerID, const T& msg) {
                std::invoke(f, this, msg);
            });
    }
};