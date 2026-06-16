#pragma once
#include "common/entity.hpp"
#include "common/manager.hpp"

class ReplicateComponent {
public:
    explicit ReplicateComponent(Entity entity);
    Entity GetRawEntity() const;

private:
    Entity m_raw_entity = null_entity;
};

class ReplicateComponentManager: public ComponentManager<ReplicateComponent> {
};