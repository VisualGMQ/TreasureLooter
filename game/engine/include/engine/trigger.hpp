#pragma once

#include "engine/manager.hpp"
#include "engine/physics.hpp"
#include "schema/physics_schema.hpp"

class TriggerEnterEvent {
public:
    explicit TriggerEnterEvent(TriggerEventType, OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;

private:
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class TriggerLeaveEvent {
public:
    explicit TriggerLeaveEvent(TriggerEventType, OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;

private:
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class TriggerTouchEvent {
public:
    explicit TriggerTouchEvent(TriggerEventType, OverlapResult);
    [[nodiscard]] TriggerEventType GetType() const;

private:
    TriggerEventType m_type;
    OverlapResult m_overlap;
};

class Trigger {
public:
    friend class TriggerComponentManager;

    Trigger() = default;
    Trigger(Entity, const TriggerDefinition&);
    Trigger(Entity, const Circle&, TriggerEventType event_type,
            const CollisionGroup& collision_layer,
            const CollisionGroup& collision_mask);
    Trigger(Entity, const Rect&, TriggerEventType event_type,
            const CollisionGroup& collision_layer,
            const CollisionGroup& collision_mask);
    [[nodiscard]] const PhysicsActor* GetActor() const;
    ~Trigger();

    void SetEventType(TriggerEventType type);
    [[nodiscard]] TriggerEventType GetEventType() const;

    void EnableTriggerEveryFrameWhenTouch(bool);
    bool IsTriggerEveryFrameWhenTouch() const;

    void Update();

private:
    PhysicsActor* m_actor = nullptr;
    TriggerEventType m_event_type;
    bool m_trig_every_frame_when_touch = false;

    std::vector<PhysicsActor*> m_touch_actors;
};

class TriggerComponentManager : public ComponentManager<Trigger> {
public:
    void Update();
};