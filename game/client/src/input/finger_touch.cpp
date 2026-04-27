#include "client/input/finger_touch.hpp"

#include "client/context.hpp"
#include "client/window.hpp"
#include "common/log.hpp"
#include "common/profile.hpp"

bool FingerTouch::IsPressing() const {
    return m_is_last_frame_press && m_is_press;
}

bool FingerTouch::IsReleasing() const {
    return !m_is_last_frame_press && !m_is_press;
}

bool FingerTouch::IsReleased() const {
    return m_is_last_frame_press && !m_is_press;
}

bool FingerTouch::IsPressed() const {
    return !m_is_last_frame_press && m_is_press;
}

Vec2 FingerTouch::Position() const {
    return m_position * CLIENT_CONTEXT.m_window->GetWindowSize();
}

const Vec2& FingerTouch::Offset() const {
    return m_offset;
}

void FingerTouch::handleEvent(const SDL_TouchFingerEvent& event) {
    m_position.x = event.x;
    m_position.y = event.y;
    m_offset.x = event.dx;
    m_offset.y = event.dy;

    m_is_last_frame_press = m_is_press;
    if (event.type == SDL_EVENT_FINGER_DOWN) {
        m_is_press = true;
        m_last_down_time = CLIENT_CONTEXT.m_time->GetCurrentTime();
    } else if (event.type == SDL_EVENT_FINGER_CANCELED ||
               event.type == SDL_EVENT_FINGER_UP) {
        m_is_press = false;
        m_last_up_time = CLIENT_CONTEXT.m_time->GetCurrentTime();
    } else if (event.type == SDL_EVENT_FINGER_MOTION) {
        // nothing todo
    }

    m_has_handled_event = true;
}

void FingerTouch::update() {
    if (!m_has_handled_event) {
        m_is_last_frame_press = m_is_press;
    }

    m_has_handled_event = false;
}

void FingerTouch::postUpdate() {
    m_offset = Vec2{};
}

void Touches::HandleEvent(const SDL_TouchFingerEvent& event) {
    if (event.fingerID >= m_touches.size()) {
        LOGW("finger id out of range. ID = {}, max touch size = {}",
             event.fingerID, m_touches.size());
        return;
    }

    m_touches[event.fingerID].handleEvent(event);
}

void Touches::Update() {
    PROFILE_SECTION();

    for (auto& touch : m_touches) {
        touch.update();
    }
}

void Touches::PostUpdate() {
    PROFILE_SECTION();

    for (auto& touch : m_touches) {
        touch.postUpdate();
    }
}

const std::array<FingerTouch, MaxFingerNum>& Touches::GetFingers() const {
    return m_touches;
}
