#pragma once
#include "math.hpp"
#include "pch.hpp"

namespace tl::input {

class FingerManager {
public:
    class TouchPoint {
    public:
        friend class FingerManager;

        TouchPoint(const TouchPoint&) = delete;
        TouchPoint(TouchPoint&&) = delete;
        TouchPoint& operator=(const TouchPoint&) = delete;
        TouchPoint& operator=(TouchPoint&&) = delete;

        void HandleEvent(const SDL_TouchFingerEvent&);
        void Update();

        bool IsPressed() const { return isPressing_ && !isPressed_; }

        bool IsPressing() const { return isPressing_ && isPressed_; }

        bool IsReleased() const { return !isPressing_ && isPressed_; }

        bool IsReleasing() const { return !isPressing_ && !isPressed_; }

        const Vec2& GetPosition() const { return curPos_; }

        const Vec2& GetOffset() const { return offset_; }

    private:
        TouchPoint() = default;

        Vec2 curPos_;  // in [0, 1]
        Vec2 offset_;  // in [0, 1]

        float pressure_ = 0;
        bool isPressing_ = false;
        bool isPressed_ = false;
    };

    static constexpr size_t MaxFingerCount = 16;

    const TouchPoint& GetFinger(size_t idx) const { return fingers_[idx]; }

    size_t FingerMaxCount() const { return MaxFingerCount; }

    void HandleEvent(const SDL_Event&);
    void Update();

private:
    TouchPoint fingers_[MaxFingerCount];
};

}  // namespace tl