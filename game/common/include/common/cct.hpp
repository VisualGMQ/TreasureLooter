#pragma once

#include "common/manager.hpp"
#include "common/physics.hpp"
#include "schema/physics_schema.hpp"

#include <vector>

/**
 * One shape touched by the last MoveAndSlide call, with the normal of the
 * contact. Several shapes can be touched in the same call, a wall and a
 * character for example.
 */
struct TouchedShape {
    PhysicsShape* m_shape = nullptr;
    Vec2 m_normal;
};

class CharacterController {
public:
    friend class CCTManager;
    explicit CharacterController(LogicEntity entity,
                                 const CCTDefinition& create_info);

    /**
     * @return is touched other shape
     */
    bool MoveAndSlide(const Vec2& dir);
    [[nodiscard]] Vec2 GetPosition() const;

    void SetSkin(float skin);
    float GetSkin() const;
    void SetMinDisp(float);
    float GetMinDisp() const;
    void Teleport(const Vec2& pos);

    /**
     * Check last MoveAndSlide call has touched object.
     *
     * MUST call after MoveAndSlide
     */
    [[nodiscard]] bool HasTouched() const;

    /**
     * Get last MoveAndSlide call has touched object.
     *
     * MUST call after MoveAndSlide
     */
    PhysicsShape* GetTouchedShape();

    /**
     * Normal of the last touch, zero when nothing was touched. It points away
     * from the touched surface.
     *
     * MUST call after MoveAndSlide
     */
    [[nodiscard]] Vec2 GetTouchedNormal() const;

    /**
     * Every shape touched by the last MoveAndSlide call, in the order they
     * were hit.
     *
     * MUST call after MoveAndSlide
     */
    [[nodiscard]] const std::vector<TouchedShape>& GetTouchedShapes() const;

    [[nodiscard]] const PhysicsShape* GetPhysicsShape() const;
    PhysicsShape* GetPhysicsShape();

private:
    float m_skin = 0.1;
    float m_min_disp = 1;
    PhysicsShape::Proxy m_shape;
    PhysicsShape* m_touched_shape{nullptr};
    Vec2 m_touched_normal;
    std::vector<TouchedShape> m_touched_shapes;

    void recordTouchedShape(PhysicsShape* shape, const Vec2& normal);

    static constexpr uint32_t MaxIter = 10;

    static bool EnableDebugOutput;
};

class CCTManager : public ComponentManager<CharacterController> {
public:
    void Enable(LogicEntity entity) override;
    void Disable(LogicEntity entity) override;
};
