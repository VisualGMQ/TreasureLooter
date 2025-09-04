#pragma once

#include "SDL3/SDL.h"
#include "engine/input/button.hpp"
#include "engine/math.hpp"
#include "schema/mouse.hpp"

#include <array>

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
    const Vec2& GetPosition() const;
    const Vec2& GetOffset() const;

private:
    std::array<MouseButton, 5> m_buttons;
    Vec2 m_cur_position;
    Vec2 m_cur_offset;
};