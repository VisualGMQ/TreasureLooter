#pragma once
#include "engine/manager.hpp"
#include "engine/physics.hpp"
#include "schema/physics_schema.hpp"

class StaticCollision {
public:
    friend class StaticCollisionManager;

    explicit StaticCollision(Entity entity, const StaticCollisionDefinition&);
    ~StaticCollision();

private:
    struct ShapeInfo {
        Vec2 m_local_position;
        PhysicsShape* m_shape;
    };

    std::vector<ShapeInfo> m_shapes;
};

class StaticCollisionManager : public ComponentManager<StaticCollision> {
public:
    void Update();
};
