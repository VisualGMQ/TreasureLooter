#pragma once
#include "entity.hpp"
#include "manager.hpp"
#include "schema/relationship.hpp"

#include <vector>

class Transform;

class RelationshipManager : public ComponentManager<Relationship> {
public:
    explicit RelationshipManager(Entity root);

    void Update();

private:
    Entity m_root;

    void updatePoseRecursive(const Transform& parent_transform, Entity child);
};