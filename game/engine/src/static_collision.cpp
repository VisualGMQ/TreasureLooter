#include "engine/static_collision.hpp"
#include "engine/context.hpp"
#include "engine/macros.hpp"
#include "engine/math.hpp"
#include "schema/physics_schema.hpp"

StaticCollision::StaticCollision(Entity entity,
                                 const StaticCollisionDefinition& definition) {
    for (PhysicsActorDefinitionHandle collision : definition.m_collisions) {
        PhysicsActor* actor =
            CURRENT_CONTEXT.m_physics_scene->CreateActor(entity, collision);
        ActorInfo info;
        info.m_actor = actor;
        info.m_local_position = collision->m_rect.m_center;
        m_actors.push_back(info);
    }
}

StaticCollision::~StaticCollision() {
    for (auto actor : m_actors) {
        CURRENT_CONTEXT.m_physics_scene->RemoveActor(actor.m_actor);
    }
}

void StaticCollisionManager::Update() {
    for (auto& component : m_components) {
        TL_CONTINUE_IF_FALSE(component.second.m_enable);

        Transform* transform =
            CURRENT_CONTEXT.m_transform_manager->Get(component.first);
        TL_CONTINUE_IF_FALSE(transform);

        for (auto& info : component.second.m_component->m_actors) {
            Mat33 result = transform->GetGlobalMat() *
                           Mat33::CreateTranslation(info.m_local_position);
            Vec2 final_position = GetPosition(result);
            info.m_actor->MoveTo(final_position);
        }
    }
}
