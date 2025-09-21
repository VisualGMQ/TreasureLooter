#include "engine/input/keyboard.hpp"

#include "engine/context.hpp"

KeyboardButton::KeyboardButton(Key key) : m_key(key) {}

bool KeyboardButton::IsPressing() const {
    return m_is_last_frame_press && m_is_press;
}

bool KeyboardButton::IsReleasing() const {
    return !(m_is_last_frame_press && m_is_press);
}

bool KeyboardButton::IsReleased() const {
    return m_is_last_frame_press && !m_is_press;
}

bool KeyboardButton::IsPressed() const {
    return !m_is_last_frame_press && m_is_press;
}

void KeyboardButton::handleEvent(const SDL_KeyboardEvent& event) {
    if (static_cast<Key>(event.key) != m_key) {
        return;
    }

    m_is_last_frame_press = m_is_press;
    if (event.type == SDL_EVENT_KEY_DOWN) {
        m_is_press = true;
        m_last_down_time = CURRENT_CONTEXT.m_time->GetCurrentTime();
    } else {
        m_is_press = false;
        m_last_up_time = CURRENT_CONTEXT.m_time->GetCurrentTime();
    }

    m_has_handled_event = true;
}

void KeyboardButton::update() {
    if (!m_has_handled_event) {
        m_is_last_frame_press = m_is_press;
    }

    m_has_handled_event = false;
}

void Keyboard::HandleEvent(const SDL_KeyboardEvent& event) {
    if (auto it = m_buttons.find(static_cast<Key>(event.key));
        it != m_buttons.end()) {
        it->second.handleEvent(event);
    } else {
        auto button =
            m_buttons.emplace(static_cast<Key>(event.key),
                              KeyboardButton{static_cast<Key>(event.key)});
        button.first->second.handleEvent(event);
    }
}

void Keyboard::Update() {
    for (auto& [_, button] : m_buttons) {
        button.update();
    }
}

const KeyboardButton& Keyboard::Get(Key key) {
    if (auto it = m_buttons.find(key); it != m_buttons.end()) {
        return it->second;
    }
    return m_buttons.emplace(key, KeyboardButton{key}).first->second;
}
