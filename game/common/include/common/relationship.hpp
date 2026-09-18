#pragma once
#include "common/entity.hpp"
#include "common/manager.hpp"
#include "schema/relationship.hpp"

class Transform;

class Relationship {
public:
    explicit Relationship(LogicEntity);

    size_t GetChildrenCount() const;
    LogicEntity Get(size_t index) const;
    LogicEntity GetParent() const;
    void AddChild(LogicEntity);
    bool HasChildren() const;

    void RemoveChild(LogicEntity);
    void RemoveFromParent();

private:
    LogicEntity m_owner = null_entity;
    LogicEntity m_parent = null_entity;
    std::vector<LogicEntity> m_children;
};

class RelationshipManager : public ComponentManager<Relationship> {
public:
    void Update();

private:
    void updatePoseRecursive(const Transform& parent_transform, LogicEntity child,
                             bool parent_changed);
};
