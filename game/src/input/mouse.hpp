#pragma once
#include "math.hpp"
#include "pch.hpp"


namespace tl::input {

class Mouse {
public:
    class Button {
    public:
        friend class Mouse;

        enum class Type {
            Left = SDL_BUTTON_LEFT,
            Right = SDL_BUTTON_RIGHT,
            Middle = SDL_BUTTON_MIDDLE,
            X1 = SDL_BUTTON_X1,
            X2 = SDL_BUTTON_X2,
        };

        Button(const Button&) = delete;
        Button(Button&&) = delete;
        Button& operator=(const Button&) = delete;
        Button& operator=(Button&&) = delete;

        Type GetType() const { return type_; }

        bool IsPressed() const { return isPressing_ && !isPressed_; }

        bool IsPressing() const { return isPressing_ && isPressed_; }

        bool IsReleased() const { return !isPressing_ && isPressed_; }

        bool IsReleasing() const { return !isPressing_ && !isPressed_; }

        void HandleEvent(const SDL_MouseButtonEvent&);
        void Update();

    private:
        Type type_;
        bool isPressed_ = false;
        bool isPressing_ = false;

        Button() = default;
    };

    Mouse();

    const Button& GetButton(Button::Type) const;
    const Vec2& GetPosition() const;
    const Vec2& GetOffset() const;
    void HandleEvent(const SDL_Event&);
    void Update();

private:
    Button buttons_[5];
    Vec2 curPos_;
    Vec2 offset_;
};

}  // namespace tl