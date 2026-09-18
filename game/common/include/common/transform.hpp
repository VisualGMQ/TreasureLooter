#pragma once
#include "common/manager.hpp"
#include "common/math.hpp"

class TransformManager : public ComponentManager<Transform, LogicEntity> {
public:
    ~TransformManager() override { resetHierarchy(); }

    void RemoveEntity(LogicEntity entity) {
        if (auto* transform = Get(entity)) {
            transform->SetParent(nullptr);
            transform->DetachChildren();
        }
        ComponentManager<Transform, LogicEntity>::RemoveEntity(entity);
    }

    void Clear() {
        resetHierarchy();
        ComponentManager<Transform, LogicEntity>::Clear();
    }

private:
    // Drop every hierarchy link before the transforms are destroyed, so the
    // (unspecified) destruction order can't make a transform touch an already
    // destroyed parent/child.
    void resetHierarchy() {
        for (auto& [entity, component] : this->m_components) {
            if (component.m_component) {
                component.m_component->ResetHierarchy();
            }
        }
    }
};

/**
 * render only transform, stores the present entity's transform. it mirrors the
 * corresponding logic transform's global matrix.
 */
class PresentTransformManager
    : public ComponentManager<Transform, PresentEntity> {
public:
    ~PresentTransformManager() override { resetHierarchy(); }

    void RemoveEntity(PresentEntity entity) {
        if (auto* transform = Get(entity)) {
            transform->SetParent(nullptr);
            transform->DetachChildren();
        }
        ComponentManager<Transform, PresentEntity>::RemoveEntity(entity);
    }

    void Clear() {
        resetHierarchy();
        ComponentManager<Transform, PresentEntity>::Clear();
    }

private:
    void resetHierarchy() {
        for (auto& [entity, component] : this->m_components) {
            if (component.m_component) {
                component.m_component->ResetHierarchy();
            }
        }
    }
};
