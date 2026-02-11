#pragma once
#include "engine/asset_manager.hpp"
#include "engine/entity.hpp"
#include "engine/event.hpp"
#include "engine/level.hpp"
#include "engine/input/input.hpp"
#include "engine/ui.hpp"
#include "schema/common.hpp"

class ControllerAxis {
public:
    explicit ControllerAxis(SDL_JoystickID, const Axis&);

    float Value() const;

private:
    SDL_JoystickID m_joystick_id;
    const Axis& m_axis;
};

class ControllerAxises {
public:
    explicit ControllerAxises(SDL_JoystickID, const Axises& axises);

    Vec2 Value() const;

private:
    SDL_JoystickID m_joystick_id;
    Axises m_axises;
};

class ControllerAction {
public:
    explicit ControllerAction(SDL_JoystickID, const Action& action);

    bool IsPressed() const;
    bool IsPressing() const;
    bool IsReleased() const;
    bool IsReleasing() const;
    bool IsRelease() const;
    bool IsPress() const;

private:
    SDL_JoystickID m_joystick_id;
    const Action& m_action;
};

struct VirtualJoystick {
    Entity m_ui_entity = null_entity;
    Circle m_circle;
    float m_max_drag_dist = 400;
    EventListenerID m_drag_event{};
    EventListenerID m_release_event{};
};

struct VirtualButton {
    Entity m_ui_entity = null_entity;
    EventListenerID m_press_event{};
    EventListenerID m_release_event{};
    const Button* m_button{};
    std::string m_action_name;
};

class TransformManager;
class AssetsManager;
class RelationshipManager;
class GameConfig;

class PlayerController {
public:
    PlayerController(InputManager&, EventSystem&, AssetsManager&,
                     TransformManager&, RelationshipManager&);
    ~PlayerController();

    ControllerAxis GetAxis(const std::string& name) const;
    ControllerAction GetAction(const std::string& name) const;
    ControllerAxises MakeAxises(const std::string& x_name,
                                const std::string& y_name);

    void RegisterVirtualController(LevelHandle level, const GameConfig&);
    void DestroyVirtualController(LevelHandle level);

private:
    SDL_JoystickID m_joystick_id = 0;
    InputManager& m_input_mgr;
    EventSystem& m_event_system;
    AssetsManager& m_assets_mgr;
    TransformManager& m_transform_mgr;
    RelationshipManager& m_relationship_mgr;

    EventListenerID m_gamepad_event;

    // virtual controller relate
    std::optional<size_t> m_move_finger_idx;
    std::optional<size_t> m_attack_finger_idx;
    EventListenerID m_gamepad_event_listener;
    EventListenerID m_window_resize_event_listener;

    VirtualJoystick m_virtual_joystick;
    VirtualButton m_virtual_attack_button;

    void initVirualJoystick(LevelHandle level, const GameConfig&);
    void initVirualAttackButton(LevelHandle level, const GameConfig&);

    void handleJoystickDragEvent(EventListenerID id, const UIDragEvent&);
    void handleJoystickReleaseEvent(EventListenerID id, const UIMouseUpEvent&);
    void handleVirtualAttackButtonPressedEvent(EventListenerID id,
                                               const UIMouseDownEvent&);
    void handleVirtualAttackButtonReleasedEvent(EventListenerID id,
                                                const UIMouseUpEvent&);
};
