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
    LogicEntity FindChildByName(LogicEntity entity, const std::string& name);
    std::vector<LogicEntity> FindChildrenByName(LogicEntity entity, const std::string& name);
    LogicEntity Find(const std::string_view name);

private:
    void findChildByName(LogicEntity entity, const std::string& name, LogicEntity& result);
    void findChildrenByName(LogicEntity entity, const std::string& name, std::vector<LogicEntity>& result);
};

