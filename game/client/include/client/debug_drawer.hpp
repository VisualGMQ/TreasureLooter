#pragma once
#include "common/math.hpp"
#include "common/timer.hpp"
#include "schema/common.hpp"
#include "common/debug_drawer.hpp"

#include <vector>

class DebugDrawer final : public IDebugDrawer {
public:
    void DrawRect(const Rect&, const Color& color, TimeType m_time, bool use_camera = true) override;
    void DrawCircle(const Circle&, const Color& color,
                    TimeType m_time, bool use_camera = true) override;
    void FillRect(const Rect&, const Color& color, TimeType m_time, bool use_camera = true) override;
    void AddLine(const Vec2&, const Vec2&, const Color& color,
                 TimeType m_time, bool use_camera = true) override;

    void Update(TimeType) override;
    void Clear() override;

private:
    template <typename T>
    struct Element {
        Color m_color;
        T m_value;
        TimeType m_time{};
        bool use_camera = false;
    };

    std::vector<Element<Rect>> m_fill_rects;
    std::vector<Element<Circle>> m_circles;
    std::vector<Element<Rect>> m_rects;
    std::vector<Element<std::pair<Vec2, Vec2>>> m_segments;
    
    TimeType decTime(TimeType cur_time, TimeType elapse);
};
