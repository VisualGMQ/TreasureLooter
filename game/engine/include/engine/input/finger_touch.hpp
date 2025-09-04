#pragma once
#include "SDL3/SDL.h"
#include "engine/input/button.hpp"
#include "engine/math.hpp"

#include <array>

constexpr uint8_t MaxFingerNum = 20;

class FingerTouch : public Button {
public:
    friend class Touches;
    
    bool IsPressing() const override;
    bool IsReleasing() const override;
    bool IsReleased() const override;
    bool IsPressed() const override;

    const Vec2& Position() const;
    const Vec2& Offset() const;

private:
    bool m_is_press = false;
    bool m_is_last_frame_press = false;
    bool m_has_handled_event = false;
    Vec2 m_position;
    Vec2 m_offset;
    
    void handleEvent(const SDL_TouchFingerEvent&);
    void update();
    void postUpdate();
};

class Touches {
public:
    void HandleEvent(const SDL_TouchFingerEvent&);
    void Update();
    void PostUpdate();

    const std::array<FingerTouch, MaxFingerNum>& GetFingers() const;

private:
    std::array<FingerTouch, MaxFingerNum> m_touches;
};