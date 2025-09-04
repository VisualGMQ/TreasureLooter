#include "engine/collision_group.hpp"

CollisionGroup::CollisionGroup(
    std::initializer_list<CollisionGroupType>&& list) {
    for (CollisionGroupType type : list) {
        Add(type);
    }
}

void CollisionGroup::Add(CollisionGroupType group) {
    m_collision_group |= 1 << static_cast<underlying_type>(group);
}

void CollisionGroup::Remove(CollisionGroupType group) {
    m_collision_group &= ~(1 << static_cast<underlying_type>(group));
}

bool CollisionGroup::Has(CollisionGroupType group) const {
    return m_collision_group & (1 << static_cast<underlying_type>(group));
}

void CollisionGroup::Clear() {
    m_collision_group = 0;
}

bool CollisionGroup::CanCollision(CollisionGroup o) const {
    return m_collision_group & o.m_collision_group;
}

CollisionGroup::underlying_type CollisionGroup::GetUnderlying() const {
    return m_collision_group;
}

void CollisionGroup::SetUnderlying(underlying_type underlying) {
    m_collision_group = underlying;
}
