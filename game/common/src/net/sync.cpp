#include "common/net/sync.hpp"

ReplicateComponent::ReplicateComponent(Entity entity): m_raw_entity{entity} {
}

Entity ReplicateComponent::GetRawEntity() const {
    return m_raw_entity;
}