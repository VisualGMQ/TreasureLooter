#pragma once
#include "engine/math.hpp"

class Camera {
public:
    void ChangeScale(const Vec2& scale) { m_scale = scale; }

    void MoveTo(const Vec2& p) { m_position = p; }

    void Move(const Vec2& offset) { m_position += offset; }

    const Vec2& GetScale() const { return m_scale; }

    const Vec2& GetPosition() const { return m_position; }

    void transform(Vec2* center, Vec2* size) const;

private:
    Vec2 m_scale = {1, 1};
    Vec2 m_position = {0, 0};
};
