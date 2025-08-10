#pragma once
#include "SDL3/SDL.h"
#include "button.hpp"
#include "schema/gamepad.hpp"

#include <array>
#include <memory>
#include <unordered_map>

class GamepadButton : public Button {
public:
    friend class Gamepad;

    explicit GamepadButton(SDL_GamepadButton type);

    bool IsPressing() const override;
    bool IsReleasing() const override;
    bool IsReleased() const override;
    bool IsPressed() const override;

    SDL_GamepadButton GetType() const;

private:
    SDL_GamepadButton m_type;
    bool m_is_press = false;
    bool m_is_last_frame_press = false;
    bool m_has_handled_event = false;

    void handleEvent(const SDL_GamepadButtonEvent&);
    void update();
};

class GamepadAxis {
public:
    friend class Gamepad;

    explicit GamepadAxis(SDL_GamepadAxis);
    float Value() const;

    SDL_GamepadAxis GetType() const;

private:
    SDL_GamepadAxis m_axis;
    float m_value{};

    void handleEvent(const SDL_GamepadAxisEvent&);
};

class Gamepad {
public:
    friend class GamepadManager;
    
    explicit Gamepad(SDL_JoystickID id);
    SDL_JoystickID GetID() const;

    const GamepadButton& GetButton(GamepadButtonType) const;
    const GamepadAxis& GetAxis(GamepadAxisType) const;

private:
    SDL_JoystickID m_id;

    std::array<std::unique_ptr<GamepadButton>, SDL_GAMEPAD_BUTTON_COUNT>
        m_buttons;
    std::array<std::unique_ptr<GamepadAxis>, SDL_GAMEPAD_AXIS_COUNT> m_axis;

    void handleEvent(const SDL_Event&);
    void update();

    static GamepadButton InvalidButton;
    static GamepadAxis InvalidAxis;
};

class GamepadManager {
public:
    void HandleEvent(const SDL_Event&);
    void Update();

    const auto& GetGamepads() const { return m_gamepads; }
    const Gamepad* Find(SDL_JoystickID) const;

private:
    std::unordered_map<SDL_JoystickID, Gamepad> m_gamepads;
};