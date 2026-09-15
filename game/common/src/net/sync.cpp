#include "common/net/sync.hpp"

ReplicateComponent::ReplicateComponent(Entity entity) : m_raw_entity{entity} {}

Entity ReplicateComponent::GetRawEntity() const {
    return m_raw_entity;
}

void ReplicateComponent::Update(TimeType elapse_time) {

}

void ReplicateComponentManager::Update(TimeType elapse_time) {
    for (auto& [entity, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);

        component.m_component->Update(elapse_time);
    }
}