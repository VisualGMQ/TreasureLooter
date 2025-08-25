#pragma once
#include "entity.hpp"
#include "manager.hpp"
#include "schema/relationship.hpp"

class Transform;

class RelationshipManager : public ComponentManager<Relationship> {
public:
    void Update();

private:
    void updatePoseRecursive(const Transform& parent_transform, Entity child);
};