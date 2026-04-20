#pragma once

#include <cstddef>

#include "common/manager.hpp"
#include "common/physics.hpp"
#include "schema/physics_schema.hpp"

class StaticCollision {
public:
    friend class StaticCollisionManager;

    explicit StaticCollision(Entity entity, const StaticCollisionDefinition&);
    ~StaticCollision();

    [[nodiscard]] std::size_t GetPhysicsShapeCount() const noexcept {
        return m_shapes.size();
    }

    [[nodiscard]] PhysicsShape* GetPhysicsShape(std::size_t index) {
        if (index >= m_shapes.size())
            return nullptr;
        return m_shapes[index].m_shape;
    }

    [[nodiscard]] Vec2 GetPhysicsShapeLocalPosition(std::size_t index) const {
        if (index >= m_shapes.size())
            return {};
        return m_shapes[index].m_local_position;
    }

private:
    struct ShapeInfo {
        Vec2 m_local_position;
        PhysicsShape* m_shape;
    };

    std::vector<ShapeInfo> m_shapes;
};

class StaticCollisionManager : public ComponentManager<StaticCollision> {
public:
    void Enable(Entity) override;
    void Disable(Entity) override;
    void Update();
};
