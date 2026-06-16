#pragma once

#include "common/entity.hpp"
#include "common/manager.hpp"

struct EntityName {
    EntityName() = default;
    explicit EntityName(const std::string& name);
    std::string m_name;
};

class EntityNameManager: public ComponentManager<EntityName> {
public:
    Entity FindChildByName(Entity entity, const std::string& name);
    std::vector<Entity> FindChildrenByName(Entity entity, const std::string& name);

private:
    void findChildByName(Entity entity, const std::string& name, Entity& result);
    void findChildrenByName(Entity entity, const std::string& name, std::vector<Entity>& result);
};

