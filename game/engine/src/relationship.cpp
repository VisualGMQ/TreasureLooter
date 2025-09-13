#include "engine/relationship.hpp"

#include "engine/context.hpp"
#include "engine/level.hpp"
#include "engine/transform.hpp"

void RelationshipManager::Update() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity root = level->GetRootEntity();

    auto relationship = Get(root);
    if (!relationship) {
        return;
    }

    auto& transform_manager = CURRENT_CONTEXT.m_transform_manager;
    Transform* root_transform = transform_manager->Get(root);
    if (!root_transform) {
        LOGE(
            "[Component][RelationshipManager] root entity don't has transform");
        return;
    }

    root_transform->UpdateMat(nullptr);

    for (auto& child : relationship->m_children) {
        updatePoseRecursive(*root_transform, child);
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
    if (!child_relationship) {
        return;
    }

    for (auto& c : child_relationship->m_children) {
        updatePoseRecursive(*transform, c);
    }
}