#pragma once
#include "engine/entity.hpp"
#include "engine/manager.hpp"
#include "schema/relationship.hpp"

class Transform;

class Relationship {
public:
    explicit Relationship(Entity);

    size_t GetChildrenCount() const;
    Entity Get(size_t index) const;
    Entity GetParent() const;
    void AddChild(Entity);
    bool HasChildren() const;

    void RemoveChild(Entity);
    void RemoveFromParent();

private:
    Entity m_owner = null_entity;
    Entity m_parent = null_entity;
    std::vector<Entity> m_children;
};

class RelationshipManager : public ComponentManager<Relationship> {
public:
    void Update();

private:
    void updatePoseRecursive(const Transform& parent_transform, Entity child);
};
