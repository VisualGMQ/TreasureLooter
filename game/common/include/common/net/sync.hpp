#pragma once
#include "common/entity.hpp"
#include "common/manager.hpp"
#include "common/math.hpp"
#include "common/timer.hpp"
#include "schema/replicate.hpp"

class ReplicateComponent {
public:
    explicit ReplicateComponent(Entity entity);
    [[nodiscard]] Entity GetRawEntity() const;

    void ReceiveNetMsg();
    void Update(TimeType elapse_time);

private:
    Entity m_raw_entity = null_entity;

    Transform m_old_transform;
};

class ReplicateComponentManager : public ComponentManager<ReplicateComponent> {
public:
    void Update(TimeType elapse_time);
};