#include "client/camera.hpp"
#include "client/context.hpp"
#include "client/window.hpp"

void Camera::transform(Vec2* center, Vec2* size) const {
    if (center) {
        auto window_size = CLIENT_CONTEXT.m_window->GetWindowSize();
        *center = (*center - GetPosition()) * GetScale() +
                  window_size * 0.5;
    }
    if (size) {
        *size *= GetScale();
    }
}
