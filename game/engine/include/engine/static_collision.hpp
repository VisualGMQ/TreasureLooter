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
    struct ActorInfo {
        Vec2 m_local_position;
        PhysicsActor* m_actor;
    };

    std::vector<ActorInfo> m_actors;
};

class StaticCollisionManager : public ComponentManager<StaticCollision> {
public:
    void Update();
};
