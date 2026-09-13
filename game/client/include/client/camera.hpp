#pragma once
#include <optional>

#include "common/math.hpp"
#include "schema/common.hpp"

class Camera {
public:
    void ChangeScale(const Vec2& scale) { m_scale = scale; }

    void MoveTo(const Vec2& p);

    void Move(const Vec2& offset);

    void SetBoundary(const Rect& boundary);

    [[nodiscard]] const Vec2& GetScale() const { return m_scale; }

    [[nodiscard]] const Vec2& GetPosition() const { return m_position; }

    void transform(Vec2* center, Vec2* size) const;

private:
    void clampToBoundary();

    Vec2 m_scale = {1, 1};
    Vec2 m_position = {0, 0};
    std::optional<Rect> m_boundary;
};
