#pragma once
#include "button.hpp"
#include "gamepad.hpp"
#include "math.hpp"
#include "path.hpp"
#include "schema/input.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Context;

class Action {
public:
    void AddButton(const Button& button);

    bool IsPressed() const;
    bool IsPressing() const;
    bool IsReleased() const;
    bool IsReleasing() const;
    bool IsRelease() const;
    bool IsPress() const;

private:
    std::vector<const Button*> m_buttons;
};

class Axis {
public:
    void AddMapping(const Button&, float scale);
    void AddMapping(const GamepadAxis&, float scale);
    void AddMouseHorizontalMapping(float scale);
    void AddMouseVerticalMapping(float scale);

    float Value() const;

private:
    struct ButtonMapping {
        const Button* m_button{};
        float m_scale = 1.0;
    };

    struct AxisMapping {
        const GamepadAxis* m_axis{};
        float m_scale = 1.0;
    };

    struct MouseMapping {
        float m_scale = 1.0;
    };

    std::vector<ButtonMapping> m_button_mappings;
    std::vector<AxisMapping> m_axis_mappings;
    std::optional<MouseMapping> m_horizontal;
    std::optional<MouseMapping> m_vertical;
};

class Axises {
public:
    Axises(const Axis& x_axis, const Axis& y_axis);

    Vec2 Value() const;

private:
    const Axis& m_x_axis;
    const Axis& m_y_axis;
};

class InputManager {
public:
    explicit InputManager(Context& context, const Path& config_filename);

    const Axis& GetAxis(const std::string& name) const;
    const Action& GetAction(const std::string& name) const;

    Axises MakeAxises(const std::string& x_name, const std::string& y_name);

    void SetConfig(Context& context, const InputConfig& config);

private:
    std::unordered_map<std::string, Axis> m_axis_mappings;
    std::unordered_map<std::string, Action> m_action_mappings;

    void loadAxisConfig(Context& context, const InputAxisConfig&);
    void loadActionConfig(Context& context, const InputActionConfig&);

    static Action InvalidAction;
    static Axis InvalidAxis;
};