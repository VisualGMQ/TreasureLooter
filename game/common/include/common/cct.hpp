#pragma once

#include "common/manager.hpp"
#include "common/physics.hpp"
#include "schema/physics_schema.hpp"

class CharacterController {
public:
    explicit CharacterController(Entity entity,
                                 const CCTDefinition& create_info);

    void MoveAndSlide(const Vec2& dir);
    [[nodiscard]] Vec2 GetPosition() const;

    void SetSkin(float skin);
    float GetSkin() const;
    void SetMinDisp(float);
    float GetMinDisp() const;
    void Teleport(const Vec2& pos);

    [[nodiscard]] const PhysicsShape* GetPhysicsShape() const;
    PhysicsShape* GetPhysicsShape();

private:
    float m_skin = 0.1;
    float m_min_disp = 1;
    PhysicsShape::Proxy m_shape;

    static constexpr uint32_t MaxIter = 10;

    static bool EnableDebugOutput;
};

class CCTManager : public ComponentManager<CharacterController> {};
