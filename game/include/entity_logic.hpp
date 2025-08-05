#pragma once

#include "entity.hpp"
#include "manager.hpp"
#include "timer.hpp"

class EntityLogic {
public:
    virtual ~EntityLogic() = default;

    explicit EntityLogic(Entity entity) : m_entity{entity} {}

    virtual void OnInit() {}
    virtual void OnLogicUpdate(TimeType) {}
    virtual void OnRenderUpdate(TimeType) {}
    virtual void OnQuit() {}

    Entity GetEntity() const { return m_entity; }
    
private:
    Entity m_entity;
};

class EntityLogicManager : public ComponentManager<EntityLogic> {};
