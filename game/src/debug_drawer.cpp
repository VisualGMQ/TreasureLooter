#include "debug_drawer.hpp"

#include "context.hpp"

void DebugDrawer::DrawRect(const Rect& r, const Color& color, TimeType time, bool use_camera) {
    m_rects.push_back({color, r, time, use_camera});
}

void DebugDrawer::DrawCircle(const Circle& c, const Color& color,
                             TimeType time, bool use_camera) {
    m_circles.push_back({color, c, time, use_camera});
}

void DebugDrawer::FillRect(const Rect& r, const Color& color, TimeType time, bool use_camera) {
    m_fill_rects.push_back({color, r, time, use_camera});
}

void DebugDrawer::AddLine(const Vec2& p1, const Vec2& p2, const Color& color,
                          TimeType time, bool use_camera) {
    m_segments.push_back({color, std::make_pair(p1, p2), time, use_camera});
}

void DebugDrawer::Update(TimeType elapse) {
    auto& renderer = GAME_CONTEXT.m_renderer;

    size_t i = 0;

    while (i < m_rects.size()) {
        auto& elem = m_rects[i];
        renderer->DrawRect(elem.m_value, elem.m_color, elem.use_camera);
        elem.m_time -= elapse;

        if (elem.m_time <= 0) {
            m_rects.erase(m_rects.begin() + i);
            continue;
        }

        i++;
    }

    i = 0;
    while (i < m_fill_rects.size()) {
        auto& elem = m_fill_rects[i];
        renderer->FillRect(elem.m_value, elem.m_color, elem.use_camera);
        elem.m_time -= elapse;

        if (elem.m_time <= 0) {
            m_fill_rects.erase(m_fill_rects.begin() + i);
            continue;
        }

        i++;
    }

    i = 0;
    while (i < m_circles.size()) {
        auto& elem = m_circles[i];
        renderer->DrawCircle(elem.m_value, elem.m_color, 20, elem.use_camera);
        elem.m_time -= elapse;

        if (elem.m_time <= 0) {
            m_circles.erase(m_circles.begin() + i);
            continue;
        }

        i++;
    }

    i = 0;
    while (i < m_segments.size()) {
        auto& elem = m_segments[i];
        renderer->DrawLine(elem.m_value.first, elem.m_value.second,
                           elem.m_color, elem.use_camera);
        elem.m_time -= elapse;

        if (elem.m_time <= 0) {
            m_segments.erase(m_segments.begin() + i);
            continue;
        }

        i++;
    }
}

void DebugDrawer::Clear() {
    m_circles.clear();
    m_fill_rects.clear();
    m_segments.clear();
    m_rects.clear();
}