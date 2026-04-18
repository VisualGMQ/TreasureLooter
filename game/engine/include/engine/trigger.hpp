#pragma once

#include "engine/manager.hpp"
#include "engine/physics.hpp"
#include "schema/physics_schema.hpp"

class TriggerEnterEvent {
public:
    explicit TriggerEnterEvent(Entity src_entity, TriggerEventType,
                               OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] Entity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;

private:
    Entity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class TriggerLeaveEvent {
public:
    explicit TriggerLeaveEvent(Entity src_entity, TriggerEventType,
                               OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] Entity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;

private:
    Entity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class TriggerTouchEvent {
public:
    explicit TriggerTouchEvent(Entity src_entity, TriggerEventType,
                               OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] Entity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;

private:
    Entity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class Trigger {
public:
    friend class TriggerComponentManager;

    Trigger() = default;
    Trigger(Entity, const TriggerDefinition&);
    [[nodiscard]] const PhysicsShape* GetPhysicsShape() const;
    [[nodiscard]] PhysicsShape* GetPhysicsShape();
    ~Trigger();

    void SetEventType(TriggerEventType type);
    [[nodiscard]] TriggerEventType GetEventType() const;

    void EnableTriggerEveryFrameWhenTouch(bool);
    bool IsTriggerEveryFrameWhenTouch() const;

    void Update();

private:
    PhysicsShape* m_shape = nullptr;
    TriggerEventType m_event_type;
    bool m_trig_every_frame_when_touch = false;

    std::vector<PhysicsShape*> m_touch_shapes;
};

class TriggerComponentManager : public ComponentManager<Trigger> {
public:
    void Update();
};
