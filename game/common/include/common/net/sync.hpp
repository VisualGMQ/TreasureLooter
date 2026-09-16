#pragma once
#include "common/entity.hpp"
#include "common/manager.hpp"
#include "common/math.hpp"
#include "common/timer.hpp"
#include "schema/replicate.hpp"

class ReplicateComponent {
public:
    explicit ReplicateComponent(LogicEntity entity);
    [[nodiscard]] LogicEntity GetRawEntity() const;

    void ReceiveNetMsg();
    void Update(TimeType elapse_time);

private:
    LogicEntity m_raw_entity = null_entity;

    Transform m_old_transform;
};

class ReplicateComponentManager : public ComponentManager<ReplicateComponent> {
public:
    void Update(TimeType elapse_time);
};