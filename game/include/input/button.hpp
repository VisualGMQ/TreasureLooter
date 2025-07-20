#pragma once

class Button {
public:
    virtual ~Button() = default;
    virtual bool IsPressing() const = 0;
    virtual bool IsReleasing() const = 0;
    virtual bool IsReleased() const = 0;
    virtual bool IsPressed() const = 0;

    bool IsPress() const { return IsPressing() && IsPressed(); }

    bool IsRelease() const { return IsReleased() && IsReleasing(); }
};