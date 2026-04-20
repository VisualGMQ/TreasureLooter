#include "common/static_collision.hpp"
#include "common/context.hpp"
#include "common/macros.hpp"
#include "common/math.hpp"
#include "schema/physics_schema.hpp"

StaticCollision::StaticCollision(Entity entity,
                                 const StaticCollisionDefinition& definition) {
    for (PhysicsShapeDefinitionHandle collision : definition.m_collisions) {
        PhysicsShape* actor =
            COMMON_CONTEXT.m_physics_scene->CreateShape(entity, collision);
        ShapeInfo info;
        info.m_shape = actor;
        info.m_local_position = collision->m_rect.m_center;
        m_shapes.push_back(info);
    }
}

StaticCollision::~StaticCollision() {
    for (auto actor : m_shapes) {
        COMMON_CONTEXT.m_physics_scene->RemoveShape(actor.m_shape);
    }
}

void StaticCollisionManager::Update() {
    for (auto& component : m_components) {
        TL_CONTINUE_IF_FALSE(component.second.m_enable);

        Transform* transform =
            COMMON_CONTEXT.m_transform_manager->Get(component.first);
        TL_CONTINUE_IF_FALSE(transform);

        for (auto& info : component.second.m_component->m_shapes) {
            Mat33 result = transform->GetGlobalMat() *
                           Mat33::CreateTranslation(info.m_local_position);
            Vec2 final_position = GetPosition(result);
            info.m_shape->MoveTo(final_position);
        }
    }
}

void StaticCollisionManager::Enable(Entity entity) {
    auto collision = Get(entity);
    TL_RETURN_IF_NULL(collision);
    ComponentManager<StaticCollision>::Enable(entity);

    for (auto& shape : collision->m_shapes) {
        shape.m_shape->SetQueryEnable(true);
    }
}

void StaticCollisionManager::Disable(Entity entity) {
    auto collision = Get(entity);
    TL_RETURN_IF_NULL(collision);
    ComponentManager<StaticCollision>::Enable(entity);

    for (auto& shape : collision->m_shapes) {
        shape.m_shape->SetQueryEnable(false);
    }
}


