#pragma once
#include "common/logic.hpp"
#include "common/net/udp.hpp"
#include "common/timer.hpp"

#include <memory>
#include <unordered_map>

struct Plank {
    UDPPeer m_peer;
    uint32_t m_id;
    Vec2 m_position;
};

class ServerLogic: public ILogic {
public:
    void OnInit() override;
    void OnUpdate(TimeType) override;
    void OnQuit() override;

private:
    std::unordered_map<UDPPeer::ID, Plank> m_peers;
    int m_side = -1;

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
