#pragma once
#include "pch.hpp"

namespace tl::input {

class GameController {
public:
    class Button {
    public:
        friend class GameController;

        enum class Type {
            Invalid = SDL_CONTROLLER_BUTTON_INVALID,
            A = SDL_CONTROLLER_BUTTON_A,
            B = SDL_CONTROLLER_BUTTON_B,
            X = SDL_CONTROLLER_BUTTON_X,
            Y = SDL_CONTROLLER_BUTTON_Y,
            Back = SDL_CONTROLLER_BUTTON_BACK,
            Guide = SDL_CONTROLLER_BUTTON_GUIDE,
            Start = SDL_CONTROLLER_BUTTON_START,
            LeftStick = SDL_CONTROLLER_BUTTON_LEFTSTICK,
            RightStick = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
            LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
            RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
            DPad_Up = SDL_CONTROLLER_BUTTON_DPAD_UP,
            DPad_Down = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
            DPad_Left = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
            DPad_Right = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
            Misc1 = SDL_CONTROLLER_BUTTON_MISC1,
            Paddle1 = SDL_CONTROLLER_BUTTON_PADDLE1,
            Paddle2 = SDL_CONTROLLER_BUTTON_PADDLE2,
            Paddle3 = SDL_CONTROLLER_BUTTON_PADDLE3,
            Paddle4 = SDL_CONTROLLER_BUTTON_PADDLE4,
            TouchPad = SDL_CONTROLLER_BUTTON_TOUCHPAD,
        };

        Button(const Button&) = delete;
        Button(Button&&) = delete;
        Button& operator=(const Button&) = delete;
        Button& operator=(Button&&) = delete;

        Type GetType() const { return type_; }

        operator bool() const { return type_ != Type::Invalid; }

        bool IsPressed() const { return isPressing_ && !isPressed_; }

        bool IsPressing() const { return isPressing_ && isPressed_; }

        bool IsReleased() const { return !isPressing_ && isPressed_; }

        bool IsReleasing() const { return !isPressing_ && !isPressed_; }

        void HandleEvent(const SDL_ControllerButtonEvent&);
        void Update();

    private:
        Type type_ = Type::Invalid;

        bool isPressing_ = false;
        bool isPressed_ = false;

        Button() = default;
    };

    class Axis {
    public:
        friend class GameController;

        enum class Type {
            Invalid = SDL_CONTROLLER_AXIS_INVALID,
            LeftX = SDL_CONTROLLER_AXIS_LEFTX,
            LeftY = SDL_CONTROLLER_AXIS_LEFTY,
            RightX = SDL_CONTROLLER_AXIS_RIGHTX,
            RightY = SDL_CONTROLLER_AXIS_RIGHTY,
            TriggerLeft = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
            TriggerRight = SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
        };

        Axis(const Axis&) = delete;
        Axis(Axis&&) = delete;
        Axis& operator=(const Axis&) = delete;
        Axis& operator=(Axis&&) = delete;

        void HandleEvent(const SDL_ControllerAxisEvent&);

        Type GetType() const { return type_; }

        double GetValue() const { return value_; }

    private:
        Type type_ = Type::Invalid;
        double value_;  // in [-1, 1]

        Axis() = default;
    };

    explicit GameController(int index);
    ~GameController();

    const Axis& GetAxis(Axis::Type) const;
    const Button& GetButton(Button::Type) const;
    void HandleEvent(const SDL_Event&);
    void Update();
    SDL_JoystickID GetID() const;

    operator bool() const { return controller_; }

private:
    SDL_GameController* controller_;
    SDL_JoystickID instanceID_;

    Button buttons_[SDL_CONTROLLER_BUTTON_MAX];
    Axis axis_[SDL_CONTROLLER_AXIS_MAX];
};

class GameControllerManager {
public:
    static SDL_JoystickID GetJoystickID(int index);

    auto& GetControllers() const { return controllers_; }

    void HandleEvent(const SDL_Event&);
    void Update();

private:
    std::unordered_map<SDL_JoystickID, GameController> controllers_;
};

}  // namespace tl