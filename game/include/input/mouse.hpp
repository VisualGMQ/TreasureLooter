#pragma once

#include "SDL3/SDL.h"
#include "input/button.hpp"
#include "math.hpp"

#include <array>

enum class MouseButtonType {
    Left = SDL_BUTTON_LEFT,
    Right = SDL_BUTTON_RIGHT,
    Middle = SDL_BUTTON_MIDDLE,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2,
};

class MouseButton: public Button {
public:
    friend class Mouse;
    
    explicit MouseButton(MouseButtonType key);

    bool IsPressing() const override;
    bool IsReleasing() const override;
    bool IsReleased() const override;
    bool IsPressed() const override;

private:
    MouseButtonType m_type;
    bool m_is_press = false;
    bool m_is_last_frame_press = false;
    bool m_has_handled_event = false;

    void handleEvent(const SDL_MouseButtonEvent&);
    void update();
};

class Mouse {
public:
    Mouse();
    void HandleEvent(const SDL_Event&);
    void Update();
    void PostUpdate();

    const MouseButton& Get(MouseButtonType) const;
    
private:
    std::array<MouseButton, 5> m_buttons;
    Vec2 m_cur_position;
    Vec2 m_cur_offset;
};