#pragma once
#include "pch.hpp"

namespace tl::input {


class Keyboard {
public:
    class Button {
    public:
        friend class Keyboard;

        Button(const Button&) = delete;
        Button(Button&&) = delete;
        Button& operator=(const Button&) = delete;
        Button& operator=(Button&&) = delete;

        SDL_Scancode GetKey() const { return key_; }

        bool IsPressed() const { return isPressing_ && !isPressed_; }

        bool IsPressing() const { return isPressing_ && isPressed_; }

        bool IsReleased() const { return !isPressing_ && isPressed_; }

        bool IsReleasing() const { return !isPressing_ && !isPressed_; }

        void HandleEvent(const SDL_KeyboardEvent&);
        void Update();

    private:
        SDL_Scancode key_ = SDL_SCANCODE_UNKNOWN;
        bool isPressing_ = false;
        bool isPressed_ = false;

        Button() = default;
    };

    Keyboard();
    const Button& GetKey(SDL_Scancode key) const;
    void HandleEvent(const SDL_Event& event);
    void Update();

private:
    Button buttons_[SDL_NUM_SCANCODES];
};

}  // namespace tl