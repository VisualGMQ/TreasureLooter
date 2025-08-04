#include "input/input.hpp"
#include "context.hpp"
#include "input/gamepad.hpp"
#include "schema/serialize/input.hpp"

void Action::AddButton(const Button& button) {
    m_buttons.push_back(&button);
}

bool Action::IsPressed() const {
    return std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsPressed(); });
}

bool Action::IsPressing() const {
    return std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsPressing(); });
}

bool Action::IsReleased() const {
    return std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsReleased(); });
}

bool Action::IsReleasing() const {
    return std::any_of(
        m_buttons.begin(), m_buttons.end(),
        [](const Button* const button) { return button->IsReleasing(); });
}

bool Action::IsRelease() const {
    return IsReleased() || IsReleasing();
}

bool Action::IsPress() const {
    return IsPressed() || IsReleasing();
}

void Axis::AddMapping(const Button& button, float scale) {
    m_button_mappings.push_back({&button, scale});
}

void Axis::AddMapping(const GamepadAxis& axis, float scale) {
    m_axis_mappings.push_back({&axis, scale});
}

void Axis::AddMouseHorizontalMapping(float scale) {
    m_horizontal = MouseMapping{scale};
}

void Axis::AddMouseVerticalMapping(float scale) {
    m_vertical = MouseMapping{scale};
}

float Axis::Value() const {
    float value = 0;
    for (auto& mapping : m_button_mappings) {
        if (mapping.m_button->IsPress()) {
            value += mapping.m_scale;
        }
    }
    for (auto& mapping : m_axis_mappings) {
        value += mapping.m_axis->Value() * mapping.m_scale;
    }

    return value;
}

Action InputManager::InvalidAction;
Axis InputManager::InvalidAxis;

Axises::Axises(const Axis& x_axis, const Axis& y_axis)
    : m_x_axis(x_axis), m_y_axis(y_axis) {}

Vec2 Axises::Value() const {
    return Vec2{m_x_axis.Value(), m_y_axis.Value()};
}

InputManager::InputManager(Context& context, InputConfigHandle config) {
    SetConfig(context, config);
}

void InputManager::SetConfig(Context& context, InputConfigHandle config) {
    for (auto& axis : config->m_axis) {
        loadAxisConfig(context, axis);
    }

    for (auto& action : config->m_action) {
        loadActionConfig(context, action);
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

    // FIXME: will not register when gamepad not found
    auto& gamepad_manager = context.m_gamepad_manager;
    if (!gamepad_manager->GetGamepads().empty()) {
        auto& gamepad = gamepad_manager->GetGamepads().begin()->second;
        for (auto& button : config.m_gamepad_button) {
            axis.AddMapping(gamepad.GetButton(button.m_button), button.m_scale);
        }
        for (auto& gamepad_axis : config.m_gamepad_axis) {
            axis.AddMapping(gamepad.GetAxis(gamepad_axis.m_axis),
                            gamepad_axis.m_scale);
        }
    }

    m_axis_mappings.emplace(config.m_name, axis);
}

void InputManager::loadActionConfig(Context& context,
                                    const InputActionConfig& config) {
    Action action;

    auto& keyboard = context.m_keyboard;
    for (auto& key : config.m_keyboard) {
        action.AddButton(keyboard->Get(key));
    }

    // FIXME: will not register when gamepad not found
    auto& gamepad_manager = context.m_gamepad_manager;
    if (!gamepad_manager->GetGamepads().empty()) {
        auto& gamepad = gamepad_manager->GetGamepads().begin()->second;
        for (auto& key : config.m_gamepad) {
            action.AddButton(gamepad.GetButton(key));
        }
    }

    m_action_mappings.emplace(config.m_name, action);
}
