#pragma once
#include "button.hpp"
#include "gamepad.hpp"
#include "engine/math.hpp"
#include "engine/path.hpp"
#include "schema/input.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class CommonContext;

class Action {
public:
    enum class State {
        Pressed,
        Pressing,
        Released,
        Releasing,
    };

    Action() = default;
    Action(const Action&) = delete;
    Action& operator=(const Action&) = delete;
    Action(Action&&) = default;
    Action& operator=(Action&&) = default;

    void AddButton(const Button& button);
    void AddButton(GamepadButtonType);

    bool IsPressed(SDL_JoystickID id = 0) const;
    bool IsPressing(SDL_JoystickID id = 0) const;
    bool IsReleased(SDL_JoystickID id = 0) const;
    bool IsReleasing(SDL_JoystickID id = 0) const;
    bool IsRelease(SDL_JoystickID id = 0) const;
    bool IsPress(SDL_JoystickID id = 0) const;

    void AcceptFingerButton(State state);

private:
    std::vector<const Button*> m_buttons;
    std::vector<GamepadButtonType> m_gamepad_button_types;
    State m_touch_state = State::Releasing;
};

class Axis {
public:
    Axis() = default;
    Axis(const Axis&) = delete;
    Axis& operator=(const Axis&) = delete;
    Axis(Axis&&) = default;
    Axis& operator=(Axis&&) = default;

    void AddMapping(const Button&, float scale);
    void AddMapping(GamepadButtonType, float scale);
    void AddMapping(GamepadAxisType, float scale, float dead_zone = 0.01);
    void AddMouseHorizontalMapping(float scale);
    void AddMouseVerticalMapping(float scale);

    void AcceptFingerAxis(float value);

    float Value(SDL_JoystickID = 0) const;

private:
    struct ButtonMapping {
        const Button* m_button{};
        float m_scale = 1.0;
    };

    struct GamepadButtonMapping {
        GamepadButtonType m_type;
        float m_scale = 1.0;
    };

    struct AxisMapping {
        GamepadAxisType m_axis;
        float m_scale = 1.0;
        float m_dead_zone = 0.01;
    };

    struct MouseMapping {
        float m_scale = 1.0;
    };

    std::vector<ButtonMapping> m_button_mappings;
    std::vector<GamepadButtonMapping> m_gamepad_button_mappings;
    std::vector<AxisMapping> m_axis_mappings;
    std::optional<MouseMapping> m_horizontal;
    std::optional<MouseMapping> m_vertical;
    float m_finger_axis_value = 0.0f;
};

class Axises {
public:
    Axises() = default;
    Axises(const Axis& x_axis, const Axis& y_axis);

    Vec2 Value(SDL_JoystickID id = 0) const;

private:
    const Axis* m_x_axis = nullptr;
    const Axis* m_y_axis = nullptr;
};

class InputManager {
public:
    void Initialize(InputConfigHandle config, CommonContext& context);

    const Axis& GetAxis(const std::string& name) const;
    const Action& GetAction(const std::string& name) const;

    Axises MakeAxises(const std::string& x_name, const std::string& y_name);

    void SetConfig(CommonContext& context, InputConfigHandle config);

    void AcceptFingerAxisEvent(const std::string& name, float value);
    void AcceptFingerButton(const std::string& name, Action::State state);

private:
    std::unordered_map<std::string, Axis> m_axis_mappings;
    std::unordered_map<std::string, Action> m_action_mappings;

    void loadAxisConfig(CommonContext& context, const InputAxisConfig&);
    void loadActionConfig(CommonContext& context, const InputActionConfig&);

    static Action InvalidAction;
    static Axis InvalidAxis;
};