#pragma once
#include "common/context.hpp"
#include "net/net.hpp"

#include "common/event.hpp"
#include <string_view>

constexpr uint32_t PORT = 1269;
constexpr std::string_view IP = "localhost";

static const Vec2 RectHalfSize{50, 70};

Vec2 CalcPlankPos(int side);

class ILogic {
public:
    virtual ~ILogic() = default;

    virtual void OnInit() = 0;
    virtual void OnUpdate(TimeType) = 0;
    virtual void OnQuit() = 0;

protected:
    template <typename T>
    void registerNetMsgDelegate(const EventListener<NetMsg<T>>& f) const {
        COMMON_CONTEXT.m_event_system->AddListener(f);
    }
};