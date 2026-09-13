#include "client/camera.hpp"
#include "client/context.hpp"
#include "client/window.hpp"

void Camera::MoveTo(const Vec2& p) {
    m_position = p;
    clampToBoundary();
}

void Camera::Move(const Vec2& offset) {
    m_position += offset;
    clampToBoundary();
}

void Camera::SetBoundary(const Rect& boundary) {
    m_boundary = boundary;
}

void Camera::clampToBoundary() {
    if (!m_boundary) {
        return;
    }

    Vec2 window_size =
        static_cast<Vec2>(CLIENT_CONTEXT.m_window->GetWindowSize());
    Vec2 half_view = window_size / (2.0f * m_scale);

    const Rect& boundary = *m_boundary;
    Vec2 min_center = boundary.m_center - boundary.m_half_size + half_view;
    Vec2 max_center = boundary.m_center + boundary.m_half_size - half_view;

    m_position.x = min_center.x > max_center.x
                       ? boundary.m_center.x
                       : Clamp(m_position.x, min_center.x, max_center.x);
    m_position.y = min_center.y > max_center.y
                       ? boundary.m_center.y
                       : Clamp(m_position.y, min_center.y, max_center.y);
}

void Camera::transform(Vec2* center, Vec2* size) const {
    if (center) {
        Vec2 window_size =
            static_cast<Vec2>(CLIENT_CONTEXT.m_window->GetWindowSize());
        *center = (*center - GetPosition()) * GetScale() + window_size * 0.5;
    }
    if (size) {
        *size *= GetScale();
    }
}
