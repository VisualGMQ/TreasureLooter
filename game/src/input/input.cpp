#include "input/input.hpp"
#include "context.hpp"
#include "input/gamepad.hpp"
#include "schema/serialize/input.hpp"

void Action::AddButton(const Button& button) {
    m_buttons.push_back(&button);
}

void Action::AddButton(GamepadButtonType button_type) {
    m_gamepad_button_types.push_back(button_type);
}

bool Action::IsPressed(SDL_JoystickID id) const {
    bool has_pressed = std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsPressed(); });
    if (has_pressed) {
        return true;
    }

    auto gamepad = GAME_CONTEXT.m_gamepad_manager->Find(id);
    if (gamepad) {
        for (auto button : m_gamepad_button_types) {
            if (gamepad->GetButton(button).IsPressed()) {
                return true;
            }
        }
    }

    return m_touch_state == State::Pressed;
}

bool Action::IsPressing(SDL_JoystickID id) const {
    bool has_pressing = std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsPressing(); });
    if (has_pressing) {
        return true;
    }

    auto gamepad = GAME_CONTEXT.m_gamepad_manager->Find(id);
    if (gamepad) {
        for (auto button : m_gamepad_button_types) {
            if (gamepad->GetButton(button).IsPressing()) {
                return true;
            }
        }
    }

    return m_touch_state == State::Pressing;
}

bool Action::IsReleased(SDL_JoystickID id) const {
    bool has_released = std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsReleased(); });

    if (has_released) {
        return true;
    }

    auto gamepad = GAME_CONTEXT.m_gamepad_manager->Find(id);
    if (gamepad) {
        for (auto button : m_gamepad_button_types) {
            if (gamepad->GetButton(button).IsReleased()) {
                return true;
            }
        }
    }

    return m_touch_state == State::Released;
}

bool Action::IsReleasing(SDL_JoystickID id) const {
    bool has_key_releasing = std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsReleasing(); });

    auto gamepad = GAME_CONTEXT.m_gamepad_manager->Find(id);
    bool is_gamepad_button_releasing = false;
    if (gamepad) {
        for (auto button : m_gamepad_button_types) {
            if (gamepad->GetButton(button).IsReleasing()) {
                is_gamepad_button_releasing = true;
                break;
            }
        }
    }

    return m_touch_state == State::Releasing && has_key_releasing && is_gamepad_button_releasing;
}

bool Action::IsRelease(SDL_JoystickID id) const {
    return IsReleased(id) || IsReleasing(id);
}

bool Action::IsPress(SDL_JoystickID id) const {
    return IsPressed(id) || IsReleasing(id);
}

void Action::AcceptFingerButton(State state) {
    m_touch_state = state;
}

void Axis::AddMapping(const Button& button, float scale) {
    m_button_mappings.emplace_back(ButtonMapping{&button, scale});
}

void Axis::AddMapping(GamepadButtonType type, float scale) {
    m_gamepad_button_mappings.emplace_back(GamepadButtonMapping{type, scale});
}

void Axis::AddMapping(GamepadAxisType axis, float scale, float dead_zone) {
    m_axis_mappings.emplace_back(AxisMapping{axis, scale, dead_zone});
}

void Axis::AddMouseHorizontalMapping(float scale) {
    m_horizontal = MouseMapping{scale};
}

void Axis::AddMouseVerticalMapping(float scale) {
    m_vertical = MouseMapping{scale};
}

void Axis::AcceptFingerAxis(float value) {
    m_finger_axis_value = value;
}

float Axis::Value(SDL_JoystickID gamepad_id) const {
    float value = 0;
    for (auto& mapping : m_button_mappings) {
        if (mapping.m_button->IsPress()) {
            value += mapping.m_scale;
        }
    }


    if (auto gamepad = GAME_CONTEXT.m_gamepad_manager->Find(gamepad_id)) {
        for (auto& mapping : m_axis_mappings) {
            auto& axis = gamepad->GetAxis(mapping.m_axis);
            float axis_value = axis.Value();
            if (std::abs(axis_value) > mapping.m_dead_zone) {
                value += axis_value * mapping.m_scale;
            }
        }

        for (auto& mapping : m_gamepad_button_mappings) {
            auto& axis = gamepad->GetButton(mapping.m_type);
            value += axis.IsPress() * mapping.m_scale;
        }
    }

    value += m_finger_axis_value;

    return value;
}

Action InputManager::InvalidAction;
Axis InputManager::InvalidAxis;

Axises::Axises(const Axis& x_axis, const Axis& y_axis)
    : m_x_axis(x_axis), m_y_axis(y_axis) {}

Vec2 Axises::Value(SDL_JoystickID id) const {
    return Vec2{m_x_axis.Value(id), m_y_axis.Value(id)};
}

void InputManager::Initialize(InputConfigHandle config) {
    SetConfig(GAME_CONTEXT, config);
}

void InputManager::SetConfig(Context& context, InputConfigHandle config) {
    for (auto& axis : config->m_axis) {
        loadAxisConfig(context, axis);
    }

    for (auto& action : config->m_action) {
        loadActionConfig(context, action);
    }
}

void InputManager::AcceptFingerAxisEvent(const std::string& name, float value) {
    if (auto it = m_axis_mappings.find(name); it != m_axis_mappings.end()) {
        it->second.AcceptFingerAxis(value);
    }
}

void InputManager::AcceptFingerButton(const std::string& name, Action::State state) {
    if (auto it = m_action_mappings.find(name); it != m_action_mappings.end()) {
        it->second.AcceptFingerButton(state);
    }
}

const Axis& InputManager::GetAxis(const std::string& name) const {
    if (auto it = m_axis_mappings.find(name); it != m_axis_mappings.end()) {
        return it->second;
    }
    return InvalidAxis;
}

const Action& InputManager::GetAction(const std::string& name) const {
    if (auto it = m_action_mappings.find(name); it != m_action_mappings.end()) {
        return it->second;
    }
    return InvalidAction;
}

Axises InputManager::MakeAxises(const std::string& x_name,
                                const std::string& y_name) {
    return {GetAxis(x_name), GetAxis(y_name)};
}

void InputManager::loadAxisConfig(Context& context,
                                  const InputAxisConfig& config) {
    Axis axis;
    auto& keyboard = context.m_keyboard;
    for (auto& key : config.m_keyboard) {
        auto& button = keyboard->Get(key.m_key);
        axis.AddMapping(button, key.m_scale);
    }
    if (config.m_mouse_horizontal) {
        axis.AddMouseHorizontalMapping(config.m_mouse_horizontal.value());
    }
    if (config.m_mouse_vertical) {
        axis.AddMouseHorizontalMapping(config.m_mouse_vertical.value());
    }

    for (auto& button : config.m_gamepad_button) {
        axis.AddMapping(button.m_button, button.m_scale);
    }
    for (auto& gamepad_axis : config.m_gamepad_axis) {
        axis.AddMapping(gamepad_axis.m_axis, gamepad_axis.m_scale);
    }

    m_axis_mappings.emplace(config.m_name, std::move(axis));
}

void InputManager::loadActionConfig(Context& context,
                                    const InputActionConfig& config) {
    Action action;

    auto& keyboard = context.m_keyboard;
    for (auto& key : config.m_keyboard) {
        action.AddButton(keyboard->Get(key));
    }

    for (auto& key : config.m_gamepad) {
        action.AddButton(key);
    }

    m_action_mappings.emplace(config.m_name, std::move(action));
}
