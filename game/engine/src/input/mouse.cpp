#include "engine/input/mouse.hpp"

#include "engine/context.hpp"
#include "engine/profile.hpp"

MouseButton::MouseButton(MouseButtonType type) : m_type(type) {}

bool MouseButton::IsPressing() const {
    return m_is_last_frame_press && m_is_press;
}

bool MouseButton::IsReleasing() const {
    return !(m_is_last_frame_press && m_is_press);
}

bool MouseButton::IsReleased() const {
    return m_is_last_frame_press && !m_is_press;
}

bool MouseButton::IsPressed() const {
    return !m_is_last_frame_press && m_is_press;
}

void MouseButton::handleEvent(const SDL_MouseButtonEvent& event) {
    if (event.button != static_cast<uint8_t>(m_type)) {
        return;
    }

    m_is_last_frame_press = m_is_press;
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        m_is_press = true;
        m_last_down_time = CURRENT_CONTEXT.m_time->GetCurrentTime();
    } else {
        m_is_press = false;
        m_last_up_time = CURRENT_CONTEXT.m_time->GetCurrentTime();
    }

    m_has_handled_event = true;
}

void MouseButton::update() {
    if (!m_has_handled_event) {
        m_is_last_frame_press = m_is_press;
    }

    m_has_handled_event = false;
}

const MouseButton& Mouse::Get(MouseButtonType type) const {
    return m_buttons[static_cast<uint8_t>(type) - 1];
}

const Vec2& Mouse::Position() const {
    return m_cur_position;
}

const Vec2& Mouse::Offset() const {
    return m_cur_offset;
}

float Mouse::Wheel() const {
    return m_wheel;
}

bool Mouse::IsTouch() const {
    return m_is_touch;
}

Mouse::Mouse()
    : m_buttons{MouseButton{MouseButtonType::Left},
                MouseButton{MouseButtonType::Middle},
                MouseButton{MouseButtonType::Right},
                MouseButton{MouseButtonType::X1},
                MouseButton{MouseButtonType::X2}} {}

void Mouse::HandleEvent(const SDL_Event& event) {
    m_is_touch = false;
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.which == SDL_TOUCH_MOUSEID) {
            m_is_touch = true;
        }
        for (auto& button : m_buttons) {
            button.handleEvent(event.button);
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        m_cur_position.x = event.motion.x;
        m_cur_position.y = event.motion.y;
        m_cur_offset.x = event.motion.xrel;
        m_cur_offset.y = event.motion.yrel;
        if (event.button.which == SDL_TOUCH_MOUSEID) {
            m_is_touch = true;
        }
    } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        m_wheel = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1) *
                  event.wheel.y;
        if (event.button.which == SDL_TOUCH_MOUSEID) {
            m_is_touch = true;
        }
    }
}

void Mouse::Update() {
    PROFILE_INPUT_SECTION(__FUNCTION__);
    
    for (auto& button : m_buttons) {
        button.update();
    }
}

void Mouse::PostUpdate() {
    m_cur_offset = Vec2{};
    m_wheel = 0;
}