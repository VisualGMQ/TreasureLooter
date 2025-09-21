#pragma once
#include "engine/timer.hpp"

class Button {
public:
    virtual ~Button() = default;
    virtual bool IsPressing() const = 0;
    virtual bool IsReleasing() const = 0;
    virtual bool IsReleased() const = 0;
    virtual bool IsPressed() const = 0;

    TimeType GetLastDownTime() const { return m_last_down_time; }

    TimeType GetLastUpTime() const { return m_last_up_time; }

    bool IsPress() const { return IsPressing() || IsPressed(); }

    bool IsRelease() const { return IsReleased() || IsReleasing(); }

protected:
    TimeType m_last_down_time{};
    TimeType m_last_up_time{};
};