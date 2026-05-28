#pragma once
#include "common/context.hpp"
#include "common/net.hpp"

#include "common/event.hpp"
#include <string_view>

constexpr uint32_t PORT = 1269;
constexpr std::string_view IP = "localhost";

static const Vec2 RectHalfSize{50, 70};

Vec2 CalcPlankPos(int side);

class ILogic {
public:
    virtual ~ILogic() = default;

    virtual void OnInit();
    virtual void OnUpdate(TimeType);
    virtual void OnQuit();

protected:
    ENetHost* m_host{};

    void initNetHost(ENetAddress* address, uint32_t peer_count,
                     uint32_t channel_limit, uint32_t incoming_bandwidth,
                     uint32_t outgoing_bandwidth);

    template <typename T>
    void registerNetMsgDelegate(const EventListener<NetMsg<T>>& f) const {
        COMMON_CONTEXT.m_event_system->AddListener(f);
    }
};