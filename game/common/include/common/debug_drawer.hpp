#pragma once
#include "common/timer.hpp"
#include "common/math.hpp"
#include "schema/common.hpp"

class IDebugDrawer {
public:
    static constexpr TimeType kOneFrame = -1;
    static constexpr TimeType kAlways = -2;

    virtual ~IDebugDrawer() = default;

    virtual void DrawRect(const Rect&, const Color& color, TimeType m_time,
                          bool use_camera) = 0;
    virtual void DrawCircle(const Circle&, const Color& color, TimeType m_time,
                            bool use_camera) = 0;
    virtual void FillRect(const Rect&, const Color& color, TimeType m_time,
                          bool use_camera) = 0;
    virtual void AddLine(const Vec2&, const Vec2&, const Color& color,
                         TimeType m_time, bool use_camera) = 0;
    virtual void Clear() = 0;

    virtual void Update(TimeType) = 0;
};

class TrivialDebugDrawer final : public IDebugDrawer {
public:
    void DrawRect(const Rect&, const Color& color, TimeType m_time, bool use_camera) override {}

    void DrawCircle(const Circle&, const Color& color,
                    TimeType m_time, bool use_camera) override {}

    void FillRect(const Rect&, const Color& color, TimeType m_time, bool use_camera) override {}

    void AddLine(const Vec2&, const Vec2&, const Color& color,
                 TimeType m_time, bool use_camera) override {}

    void Clear() override {}

    void Update(TimeType) override {}
};


