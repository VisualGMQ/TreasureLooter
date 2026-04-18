#include "engine/relationship.hpp"

#include "engine/context.hpp"
#include "engine/profile.hpp"
#include "engine/scene.hpp"
#include "engine/transform.hpp"

Relationship::Relationship(Entity entity) : m_owner{entity} {}

size_t Relationship::GetChildrenCount() const {
    return m_children.size();
}

Entity Relationship::GetParent() const {
    return m_parent;
}

Entity Relationship::Get(size_t index) const {
    return m_children[index];
}

void Relationship::AddChild(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);

    m_children.push_back(entity);
    relationship->m_parent = m_owner;
}

bool Relationship::HasChildren() const {
    return !m_children.empty();
}

void Relationship::RemoveChild(Entity entity) {
    auto it = std::find(m_children.begin(), m_children.end(), entity);
    if (it != m_children.end()) {
        auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(*it);
        relationship->m_parent = null_entity;
    }
    m_children.erase(it);
}

void RelationshipManager::Update() {
    PROFILE_SECTION();

    auto level = CURRENT_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_NULL(level);

    Entity root = level->GetRootEntity();

    auto relationship = Get(root);
    TL_RETURN_IF_NULL(relationship);

    auto& transform_manager = CURRENT_CONTEXT.m_transform_manager;
    Transform* root_transform = transform_manager->Get(root);
    if (!root_transform) {
        LOGE(
            "[Component][RelationshipManager] root entity don't has transform");
        return;
    }

    root_transform->UpdateMat(nullptr);

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        updatePoseRecursive(*root_transform, relationship->Get(i));
    }
}

void RelationshipManager::updatePoseRecursive(const Transform& parent_transform,
                                              Entity child) {
    auto& transform_manager = CURRENT_CONTEXT.m_transform_manager;
    Transform* transform = transform_manager->Get(child);

    if (!transform) {
        return;
    }

    transform->UpdateMat(&parent_transform);

    Relationship* child_relationship = Get(child);
    TL_RETURN_IF_NULL(child_relationship);

    for (size_t i = 0; i < child_relationship->GetChildrenCount(); i++) {
        updatePoseRecursive(*transform, child_relationship->Get(i));
    }
}
