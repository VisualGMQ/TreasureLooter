#include "input/gamepad.hpp"

#include "log.hpp"

GamepadButton Gamepad::InvalidButton{SDL_GAMEPAD_BUTTON_INVALID};
GamepadAxis Gamepad::InvalidAxis{SDL_GAMEPAD_AXIS_INVALID};

GamepadButton::GamepadButton(SDL_GamepadButton type) : m_type{type} {}

void GamepadAxis::handleEvent(const SDL_GamepadAxisEvent& event) {
    m_value = event.value > 0 ? event.value / 32768.0f : event.value / 32767.0f;
}

Gamepad::Gamepad(SDL_JoystickID id) : m_id{id} {
    for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++) {
        m_buttons[i] =
            std::make_unique<GamepadButton>(static_cast<SDL_GamepadButton>(i));
    }
    for (int i = 0; i < SDL_GAMEPAD_AXIS_COUNT; i++) {
        m_axis[i] =
            std::make_unique<GamepadAxis>(static_cast<SDL_GamepadAxis>(i));
    }
}

SDL_JoystickID Gamepad::GetID() const {
    return m_id;
}

const GamepadButton& Gamepad::GetButton(GamepadButtonType button) const {
    if (button == GamepadButtonType::Invalid) {
        return InvalidButton;
    }
    return *m_buttons[static_cast<size_t>(button)];
}

const GamepadAxis& Gamepad::GetAxis(GamepadAxisType axis) const {
    if (axis == GamepadAxisType::Invalid) {
        return InvalidAxis;
    }
    return *m_axis[static_cast<size_t>(axis)];
}

bool GamepadButton::IsPressing() const {
    return m_is_last_frame_press && m_is_press;
}

bool GamepadButton::IsReleasing() const {
    return !(m_is_last_frame_press && m_is_press);
}

bool GamepadButton::IsReleased() const {
    return m_is_last_frame_press && !m_is_press;
}

bool GamepadButton::IsPressed() const {
    return !m_is_last_frame_press && m_is_press;
}

SDL_GamepadButton GamepadButton::GetType() const {
    return m_type;
}

void GamepadButton::handleEvent(const SDL_GamepadButtonEvent& event) {
    if (event.button != m_type) {
        return;
    }

    m_is_last_frame_press = m_is_press;
    m_is_press = event.down;

    m_has_handled_event = true;
}

void GamepadButton::update() {
    if (!m_has_handled_event) {
        m_is_last_frame_press = m_is_press;
    }

    m_has_handled_event = false;
}

GamepadAxis::GamepadAxis(SDL_GamepadAxis axis) : m_axis{axis} {}

float GamepadAxis::Value() const {
    return m_value;
}

SDL_GamepadAxis GamepadAxis::GetType() const {
    return m_axis;
}

void Gamepad::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP ||
        event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        m_buttons[event.gbutton.button]->handleEvent(event.gbutton);
    } else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        m_axis[event.gaxis.axis]->handleEvent(event.gaxis);
    }
}

void Gamepad::update() {
    for (auto& button : m_buttons) {
        button->update();
    }
}

void GamepadManager::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        m_gamepads.emplace(event.gdevice.which, event.gdevice.which);
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        m_gamepads.erase(event.gdevice.which);
    } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP ||
               event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
               event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        for (auto& [_, gamepad] : m_gamepads) {
            gamepad.handleEvent(event);
        }
    }
}

void GamepadManager::Update() {
    for (auto& [_, gamepad] : m_gamepads) {
        gamepad.update();
    }
}

const Gamepad* GamepadManager::Find(SDL_JoystickID id) const {
    if (auto it = m_gamepads.find(id); it != m_gamepads.end()) {
        return &it->second;
    }
    return nullptr;
}