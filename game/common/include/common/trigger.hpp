#pragma once

#include "common/manager.hpp"
#include "common/physics.hpp"
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

    struct PhysicsData {
        PhysicsShape::Proxy m_shape;
        Vec2 m_local_position;
    };

    Trigger() = default;
    Trigger(Entity, const TriggerDefinition&);
    [[nodiscard]] const std::vector<PhysicsData>& GetPhysicsData() const;
    [[nodiscard]] std::vector<PhysicsData>& GetPhysicsData();

    const std::vector<PhysicsShape*>& GetTouchingShapes();

    // for debug
    std::vector<PhysicsShape*> GetUnderlyingShapes() const;

    void SetEventType(TriggerEventType type);
    [[nodiscard]] TriggerEventType GetEventType() const;

    void EnableTriggerEveryFrameWhenTouch(bool);
    bool IsTriggerEveryFrameWhenTouch() const;

    void Update();

private:
    Entity m_entity = null_entity;
    std::vector<PhysicsData> m_physics_data;
    TriggerEventType m_event_type;
    bool m_trig_every_frame_when_touch = false;

    std::vector<PhysicsShape*> m_touch_shapes;
};

class TriggerComponentManager : public ComponentManager<Trigger> {
public:
    void Update();
    void Enable(Entity) override;
    void Disable(Entity) override;

private:
    void updatePhysicsShapePosition(const Transform& parent_global_transform,
                                    Trigger::PhysicsData& physics_data) const;
};
