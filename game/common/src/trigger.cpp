#include "common/trigger.hpp"

#include "common/context.hpp"
#include "common/event.hpp"
#include "common/macros.hpp"
#include "common/manager.hpp"
#include "common/math.hpp"
#include "common/profile.hpp"

constexpr bool NullTriggerID::operator==(TriggerID id) const {
    return static_cast<std::underlying_type_t<TriggerID>>(id) == 0;
}

constexpr bool NullTriggerID::operator!=(TriggerID id) const {
    return !(*this == id);
}

constexpr bool NullTriggerID::operator==(NullTriggerID) const {
    return true;
}

constexpr bool NullTriggerID::operator!=(NullTriggerID) const {
    return false;
}

NullTriggerID::operator TriggerID() const {
    return static_cast<TriggerID>(0);
}

TriggerEnterEvent::TriggerEnterEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap,
                                     TriggerID trigger_id)
    : m_src_entity{src_entity},
      m_type{type},
      m_overlap{overlap},
      m_trigger_id{trigger_id} {}

TriggerEventType TriggerEnterEvent::GetType() const {
    return m_type;
}

Entity TriggerEnterEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerEnterEvent::GetOverlapResult() const {
    return m_overlap;
}

TriggerID TriggerEnterEvent::GetTriggerID() const {
    return m_trigger_id;
}

TriggerTouchEvent::TriggerTouchEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap,
                                     TriggerID trigger_id)
    : m_src_entity{src_entity},
      m_type{type},
      m_overlap{overlap},
      m_trigger_id{trigger_id} {}

TriggerEventType TriggerLeaveEvent::GetType() const {
    return m_type;
}

Entity TriggerLeaveEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerLeaveEvent::GetOverlapResult() const {
    return m_overlap;
}

TriggerID TriggerLeaveEvent::GetTriggerID() const {
    return m_trigger_id;
}

TriggerLeaveEvent::TriggerLeaveEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap,
                                     TriggerID trigger_id)
    : m_src_entity{src_entity},
      m_type{type},
      m_overlap{overlap},
      m_trigger_id{trigger_id} {}

TriggerEventType TriggerTouchEvent::GetType() const {
    return m_type;
}

Entity TriggerTouchEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerTouchEvent::GetOverlapResult() const {
    return m_overlap;
}

TriggerID TriggerTouchEvent::GetTriggerID() const {
    return m_trigger_id;
}

std::underlying_type_t<TriggerID> Trigger::trigger_id_counter =
    static_cast<std::underlying_type_t<TriggerID>>(
        static_cast<TriggerID>(null_trigger_id));

Trigger::Trigger(Entity entity, const TriggerDefinition& definition)
    : m_event_type{definition.m_event_type},
      m_entity{entity},
      m_trig_every_frame_when_touch{definition.m_trig_every_frame_when_touch},
      m_id{static_cast<TriggerID>(++trigger_id_counter)} {
    for (auto& shape_definition : definition.m_physics_shapes) {
        TL_CONTINUE_IF_FALSE(shape_definition);
        PhysicsData data;
        data.m_shape =
            PhysicsShape::Proxy{COMMON_CONTEXT.m_physics_scene->CreateShape(
                entity, shape_definition)};
        data.m_shape->SetQueryEnable(false);
        if (shape_definition->m_is_rect) {
            data.m_local_position = shape_definition->m_rect.m_center;
        } else {
            data.m_local_position = shape_definition->m_circle.m_center;
        }
        m_physics_data.push_back(std::move(data));
    }

    if (const Transform* transform =
            COMMON_CONTEXT.m_transform_manager->Get(entity)) {
        MoveTo(*transform);
    }
}

Trigger::Trigger(const TriggerDefinition& definition)
    : Trigger{null_entity, definition} {}

Trigger::~Trigger() {
    COMMON_CONTEXT.m_event_system->RemoveListener<TriggerEnterEvent>(
        m_enter_listener_id);
    COMMON_CONTEXT.m_event_system->RemoveListener<TriggerLeaveEvent>(
        m_leave_listener_id);
    COMMON_CONTEXT.m_event_system->RemoveListener<TriggerTouchEvent>(
        m_touch_listener_id);
}

const std::vector<Trigger::PhysicsData>& Trigger::GetPhysicsData() const {
    return m_physics_data;
}

std::vector<Trigger::PhysicsData>& Trigger::GetPhysicsData() {
    return m_physics_data;
}

const std::vector<PhysicsShape*>& Trigger::GetTouchingShapes() {
    return m_touch_shapes;
}

std::vector<PhysicsShape*> Trigger::GetUnderlyingShapes() const {
    std::vector<PhysicsShape*> shapes;
    for (auto& data : m_physics_data) {
        shapes.push_back(data.m_shape.get());
    }
    return shapes;
}

void Trigger::SetEventType(TriggerEventType type) {
    m_event_type = type;
}

TriggerEventType Trigger::GetEventType() const {
    return m_event_type;
}

void Trigger::EnableTriggerEveryFrameWhenTouch(bool enable) {
    m_trig_every_frame_when_touch = enable;
}

bool Trigger::IsTriggerEveryFrameWhenTouch() const {
    return m_trig_every_frame_when_touch;
}

Entity Trigger::GetOwner() const {
    return m_entity;
}

TriggerID Trigger::GetID() const {
    return m_id;
}

void Trigger::SetEnterListener(EventListenerID id) {
    m_enter_listener_id = id;
}

void Trigger::SetLeaveListener(EventListenerID id) {
    m_leave_listener_id = id;
}

void Trigger::SetTouchListener(EventListenerID id) {
    m_touch_listener_id = id;
}

void Trigger::SetEnterListener(const EnterListener& listener) {
    m_enter_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<TriggerEnterEvent>(
            [listener, this](EventListenerID id,
                             const TriggerEnterEvent& event) {
                TL_RETURN_IF_FALSE(id == this->m_enter_listener_id);
                listener(event);
            });
}

void Trigger::SetLeaveListener(const LeaveListener& listener) {
    m_leave_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<TriggerLeaveEvent>(
            [listener, this](EventListenerID id,
                             const TriggerLeaveEvent& event) {
                TL_RETURN_IF_FALSE(id == this->m_leave_listener_id);
                listener(event);
            });
}

void Trigger::SetTouchListener(const TouchListener& listener) {
    m_touch_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<TriggerTouchEvent>(
            [listener, this](EventListenerID id,
                             const TriggerTouchEvent& event) {
                TL_RETURN_IF_FALSE(id == this->m_touch_listener_id);
                listener(event);
            });
}

void Trigger::Enable() const {
    COMMON_CONTEXT.m_trigger_component_manager->Enable(m_entity);
}

void Trigger::Disable() const {
    COMMON_CONTEXT.m_trigger_component_manager->Disable(m_entity);
}

void Trigger::MoveTo(const Transform& transform) {
    for (auto& physics_data : m_physics_data) {
        TL_RETURN_IF_FALSE(physics_data);
        auto mat = Mat33::CreateTranslation(physics_data.m_local_position);
        auto final_mat = transform.GetGlobalMat() * mat;
        Vec2 final_position = GetPosition(final_mat);
        physics_data.m_shape->MoveTo(final_position);
    }
}

void Trigger::Update() {
    TL_RETURN_IF_TRUE(m_physics_data.empty());

    // Leave when a tracked shape no longer overlaps any of this trigger's
    // shapes.
    for (int i = static_cast<int>(m_touch_shapes.size()) - 1; i >= 0; --i) {
        PhysicsShape* target_shape = m_touch_shapes[static_cast<size_t>(i)];
        bool still_overlaps = false;
        for (auto& data : m_physics_data) {
            if (data.m_shape && COMMON_CONTEXT.m_physics_scene->Overlap(
                                    *data.m_shape, *target_shape)) {
                still_overlaps = true;
                break;
            }
        }
        if (!still_overlaps) {
            OverlapResult result;
            result.m_dst_entity = target_shape->GetOwner();
            result.m_dst_shape = target_shape;
            TriggerLeaveEvent event{m_entity, GetEventType(), result, m_id};
            COMMON_CONTEXT.m_event_system->EnqueueEvent(event);
            m_touch_shapes.erase(m_touch_shapes.begin() + i);
        }
    }

    // send still touch shape
    if (m_trig_every_frame_when_touch) {
        for (auto shape : m_touch_shapes) {
            OverlapResult result;
            result.m_dst_entity = shape->GetOwner();
            result.m_dst_shape = shape;
            TriggerTouchEvent event{m_entity, GetEventType(), result, m_id};
            COMMON_CONTEXT.m_event_system->EnqueueEvent(event);
        }
    }

    // check entered shapes
    for (auto& data : m_physics_data) {
        OverlapResult results[16];
        uint32_t count = COMMON_CONTEXT.m_physics_scene->Overlap(
            *data.m_shape, results, std::size(results));
        for (size_t i = 0; i < count; i++) {
            auto& result = results[i];

            TL_CONTINUE_IF_FALSE(result.m_dst_entity != null_entity &&
                                 result.m_dst_shape)

            auto it = std::find(m_touch_shapes.begin(), m_touch_shapes.end(),
                                result.m_dst_shape);
            if (it == m_touch_shapes.end()) {
                m_touch_shapes.push_back(result.m_dst_shape);
                TriggerEnterEvent event{m_entity, GetEventType(), result, m_id};
                COMMON_CONTEXT.m_event_system->EnqueueEvent(event);
            }
        }
    }
}

void TriggerComponentManager::Update() {
    PROFILE_SECTION();

    for (auto& [entity, trigger] : m_components) {
        TL_CONTINUE_IF_FALSE(trigger.m_enable &&
                             !trigger.m_component->m_physics_data.empty());

        auto transform = COMMON_CONTEXT.m_transform_manager->Get(entity);
        TL_CONTINUE_IF_FALSE(transform);

        trigger.m_component->MoveTo(*transform);
    }

    for (auto& [entity, trigger] : m_components) {
        TL_CONTINUE_IF_FALSE(trigger.m_enable &&
                             !trigger.m_component->m_physics_data.empty());

        trigger.m_component->Update();
    }
}

void TriggerComponentManager::Enable(Entity entity) {
    auto trigger = Get(entity);
    TL_RETURN_IF_NULL(trigger);

    ComponentManager<Trigger>::Enable(entity);
    for (auto& data : trigger->m_physics_data) {
        data.m_shape->SetQueryEnable(true);
    }
}

void TriggerComponentManager::Disable(Entity entity) {
    auto trigger = Get(entity);
    TL_RETURN_IF_NULL(trigger);

    ComponentManager<Trigger>::Disable(entity);
    for (auto& data : trigger->m_physics_data) {
        data.m_shape->SetQueryEnable(false);
    }
}

void TriggerComponentManager::ToggleDebugDraw() {
    m_enable_debug_draw = !m_enable_debug_draw;
}

void TriggerComponentManager::RenderDebug() const {
    TL_RETURN_IF_FALSE(m_enable_debug_draw);

    Color color = Color::Green;
    for (auto& components : m_components) {
        if (!components.second.m_enable) {
            color = Color::Gray;
        }

        auto& trigger = components.second.m_component;
        auto shapes = trigger->GetUnderlyingShapes();
        for (auto shape : shapes) {
            COMMON_CONTEXT.m_physics_scene->RenderShape(*shape, color);
        }
    }
}

void TriggerComponentManager::updatePhysicsShapePosition(
    const Transform& parent_global_transform,
    Trigger::PhysicsData& physics_data) const {
    TL_RETURN_IF_FALSE(physics_data.m_shape);
    auto mat = Mat33::CreateTranslation(physics_data.m_local_position);
    auto final_mat = parent_global_transform.GetGlobalMat() * mat;
    Vec2 final_position = GetPosition(final_mat);
    physics_data.m_shape->MoveTo(final_position);
}
