#include "relationship.hpp"

#include "context.hpp"
#include "transform.hpp"

RelationshipManager::RelationshipManager(Entity root) : m_root{root} {
    RegisterEntity(root);
}

void RelationshipManager::Update() {
    auto relationship = Get(m_root);
    if (!relationship) {
        return;
    }

    auto& transform_manager = Context::GetInst().m_transform_manager;
    Transform* root_transform = transform_manager->Get(m_root);
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
    auto& transform_manager = Context::GetInst().m_transform_manager;
    Transform* transform = transform_manager->Get(child);

    if (!transform) {
        LOGE("[Component][RelationshipManager] entity {} don't has transform",
             child);
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