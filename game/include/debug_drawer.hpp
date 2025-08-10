#pragma once
#include "math.hpp"
#include "schema/common.hpp"
#include "timer.hpp"

#include <vector>

class IDebugDrawer {
public:
    virtual ~IDebugDrawer() = default;

    virtual void DrawRect(const Rect&, const Color& color, TimeType m_time) = 0;
    virtual void DrawCircle(const Circle&, const Color& color,
                            TimeType m_time) = 0;
    virtual void FillRect(const Rect&, const Color& color, TimeType m_time) = 0;
    virtual void AddLine(const Vec2&, const Vec2&, const Color& color,
                         TimeType m_time) = 0;
    virtual void Clear() = 0;

    virtual void Update(TimeType) = 0;
};

class TrivialDebugDrawer final : public IDebugDrawer {
public:
    void DrawRect(const Rect&, const Color& color, TimeType m_time) override {}

    void DrawCircle(const Circle&, const Color& color,
                    TimeType m_time) override {}

    void FillRect(const Rect&, const Color& color, TimeType m_time) override {}

    void AddLine(const Vec2&, const Vec2&, const Color& color,
                 TimeType m_time) override {}

    void Clear() override {}

    void Update(TimeType) override {}
};

class DebugDrawer final : public IDebugDrawer {
public:
    void DrawRect(const Rect&, const Color& color, TimeType m_time) override;
    void DrawCircle(const Circle&, const Color& color,
                    TimeType m_time) override;
    void FillRect(const Rect&, const Color& color, TimeType m_time) override;
    void AddLine(const Vec2&, const Vec2&, const Color& color,
                 TimeType m_time) override;

    void Update(TimeType) override;
    void Clear() override;

private:
    template <typename T>
    struct Element {
        Color m_color;
        T m_value;
        TimeType m_time{};
    };

    std::vector<Element<Rect>> m_fill_rects;
    std::vector<Element<Circle>> m_circles;
    std::vector<Element<Rect>> m_rects;
    std::vector<Element<std::pair<Vec2, Vec2>>> m_segments;
};