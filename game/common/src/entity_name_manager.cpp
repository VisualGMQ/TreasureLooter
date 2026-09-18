#include "common/entity_name_manager.hpp"

#include "common/context.hpp"
#include "common/relationship.hpp"

EntityName::EntityName(const std::string& name): m_name{name} {}

LogicEntity EntityNameManager::FindChildByName(LogicEntity entity,
                                          const std::string& name) {
    LogicEntity result = null_entity;
    findChildByName(entity, name, result);
    return result;
}

std::vector<LogicEntity> EntityNameManager::FindChildrenByName(
    LogicEntity entity, const std::string& name) {
    std::vector<LogicEntity> result;
    findChildrenByName(entity, name, result);
    return result;
}

void EntityNameManager::findChildByName(LogicEntity entity, const std::string& name,
                                        LogicEntity& result) {
    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        LogicEntity child = relationship->Get(i);

        auto name_component = Get(child);

        if (name_component && name_component->m_name == name) {
            result = child;
            return;
        }

        findChildByName(child, name, result);
    }
}

void EntityNameManager::findChildrenByName(LogicEntity entity,
                                           const std::string& name,
                                           std::vector<LogicEntity>& result) {
    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        LogicEntity child = relationship->Get(i);

        auto name_component = Get(child);
        if (name_component && name_component->m_name == name) {
            result.push_back(child);
        }

        findChildrenByName(child, name, result);
    }
}
