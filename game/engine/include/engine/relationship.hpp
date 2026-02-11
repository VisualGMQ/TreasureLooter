#pragma once
#include "engine/entity.hpp"
#include "engine/manager.hpp"
#include "schema/relationship.hpp"

class Transform;

using Relationship = RelationshipDefinition;

class RelationshipManager : public ComponentManager<Relationship> {
public:
    void Update();

private:
    void updatePoseRecursive(const Transform& parent_transform, Entity child);
};