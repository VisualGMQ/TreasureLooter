#pragma once

#include "manager.hpp"
#include "physics.hpp"
#include "schema/physics.hpp"

class CharacterController {
public:
    explicit CharacterController(const CCT& create_info);

    void MoveAndSlide(const Vec2& dir);
    const Vec2& GetPosition() const;

    void SetSkin(float skin);
    void SetMinDisp(float);
    void Teleport(const Vec2& pos);

    const Circle& GetCircle() const;

private:
    Circle m_circle;
    float m_skin = 0.1;
    float m_min_disp = 1;

    static constexpr uint32_t MaxIter = 10;
};

class CCTManager : public ComponentManager<CharacterController> {
public:
    bool IsEnableDebugDraw() const;

    void ToggleDebugDraw();

    void RenderDebug();

private:
    bool m_enable_debug_draw = false;
};