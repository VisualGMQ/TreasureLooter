#pragma once

#include "common/event.hpp"
#include "common/manager.hpp"
#include "common/physics.hpp"
#include "schema/physics_schema.hpp"

enum class TriggerID : uint32_t {};

struct NullTriggerID {
    constexpr bool operator==(TriggerID) const;
    constexpr bool operator!=(TriggerID) const;
    constexpr bool operator==(NullTriggerID) const;
    constexpr bool operator!=(NullTriggerID) const;

    operator TriggerID() const;
};

constexpr NullTriggerID null_trigger_id;

class TriggerEnterEvent {
public:
    explicit TriggerEnterEvent(LogicEntity src_entity, TriggerEventType,
                               OverlapResult, TriggerID);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] LogicEntity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;
    [[nodiscard]] TriggerID GetTriggerID() const;

private:
    LogicEntity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
    TriggerID m_trigger_id = null_trigger_id;
};

class TriggerLeaveEvent {
public:
    explicit TriggerLeaveEvent(LogicEntity src_entity, TriggerEventType,
                               OverlapResult, TriggerID);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] LogicEntity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;
    [[nodiscard]] TriggerID GetTriggerID() const;

private:
    LogicEntity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
    TriggerID m_trigger_id = null_trigger_id;
};

class TriggerTouchEvent {
public:
    explicit TriggerTouchEvent(LogicEntity src_entity, TriggerEventType,
                               OverlapResult, TriggerID);
    [[nodiscard]] TriggerEventType GetType() const;
    [[nodiscard]] LogicEntity GetSrcEntity() const;
    [[nodiscard]] const OverlapResult& GetOverlapResult() const;
    [[nodiscard]] TriggerID GetTriggerID() const;

private:
    LogicEntity m_src_entity = null_entity;
    TriggerEventType m_type;
    OverlapResult m_overlap;
    TriggerID m_trigger_id = null_trigger_id;
};

class Trigger {
public:
    friend class TriggerComponentManager;

    using EnterListener = std::function<void(const TriggerEnterEvent&)>;
    using LeaveListener = std::function<void(const TriggerLeaveEvent&)>;
    using TouchListener = std::function<void(const TriggerTouchEvent&)>;

    struct PhysicsData {
        PhysicsShape::Proxy m_shape;
        Vec2 m_local_position;

        explicit operator bool() const {
            return m_shape != nullptr;
        }
    };

    Trigger() = default;
    Trigger(LogicEntity, const TriggerDefinition&);
    explicit Trigger(const TriggerDefinition&);
    ~Trigger();
    [[nodiscard]] const std::vector<PhysicsData>& GetPhysicsData() const;
    [[nodiscard]] std::vector<PhysicsData>& GetPhysicsData();

    const std::vector<PhysicsShape*>& GetTouchingShapes();

    // for debug
    [[nodiscard]] std::vector<PhysicsShape*> GetUnderlyingShapes() const;

    void SetEventType(TriggerEventType type);
    [[nodiscard]] TriggerEventType GetEventType() const;

    void EnableTriggerEveryFrameWhenTouch(bool);
    [[nodiscard]] bool IsTriggerEveryFrameWhenTouch() const;
    [[nodiscard]] LogicEntity GetOwner() const;

    void Enable() const;
    void Disable() const;

    void MoveTo(const Transform&);
    [[nodiscard]] TriggerID GetID() const;

    void SetEnterListener(const EnterListener&);
    void SetLeaveListener(const LeaveListener&);
    void SetTouchListener(const TouchListener&);

    void SetEnterListener(EventListenerID);
    void SetLeaveListener(EventListenerID);
    void SetTouchListener(EventListenerID);

    void Update();

private:
    static std::underlying_type_t<TriggerID> trigger_id_counter;

    LogicEntity m_entity = null_entity;
    TriggerID m_id = null_trigger_id;
    std::vector<PhysicsData> m_physics_data;
    TriggerEventType m_event_type = TriggerEventType::None;
    bool m_trig_every_frame_when_touch = false;

    std::vector<PhysicsShape*> m_touch_shapes;

    EventListenerID m_enter_listener_id = null_event_listener_id;
    EventListenerID m_leave_listener_id = null_event_listener_id;
    EventListenerID m_touch_listener_id = null_event_listener_id;
};

class TriggerComponentManager : public ComponentManager<Trigger> {
public:
    void Update();
    void Enable(LogicEntity) override;
    void Disable(LogicEntity) override;

    void ToggleDebugDraw();
    void RenderDebug() const;

private:
    bool m_enable_debug_draw = false;

    void updatePhysicsShapePosition(const Transform& parent_global_transform,
                                    Trigger::PhysicsData& physics_data) const;
};
