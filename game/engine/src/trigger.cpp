#include "engine/trigger.hpp"

#include "engine/context.hpp"
#include "engine/macros.hpp"
#include "engine/profile.hpp"

TriggerEnterEvent::TriggerEnterEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap)
    : m_src_entity{src_entity}, m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerEnterEvent::GetType() const {
    return m_type;
}

Entity TriggerEnterEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerEnterEvent::GetOverlapResult() const {
    return m_overlap;
}

TriggerTouchEvent::TriggerTouchEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap)
    : m_src_entity{src_entity}, m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerLeaveEvent::GetType() const {
    return m_type;
}

Entity TriggerLeaveEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerLeaveEvent::GetOverlapResult() const {
    return m_overlap;
}

TriggerLeaveEvent::TriggerLeaveEvent(Entity src_entity, TriggerEventType type,
                                     OverlapResult overlap)
    : m_src_entity{src_entity}, m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerTouchEvent::GetType() const {
    return m_type;
}

Entity TriggerTouchEvent::GetSrcEntity() const {
    return m_src_entity;
}

const OverlapResult& TriggerTouchEvent::GetOverlapResult() const {
    return m_overlap;
}

Trigger::Trigger(Entity entity, const TriggerDefinition& info)
    : m_event_type{info.m_event_type},
      m_trig_every_frame_when_touch{info.m_trig_every_frame_when_touch} {
    m_shape = CURRENT_CONTEXT.m_physics_scene->CreateShape(
        entity, info.m_physics_shape);
}

const PhysicsShape* Trigger::GetPhysicsShape() const {
    return m_shape;
}

PhysicsShape* Trigger::GetPhysicsShape() {
    return m_shape;
}

Trigger::~Trigger() {
    CURRENT_CONTEXT.m_physics_scene->RemoveShape(m_shape);
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

void Trigger::Update() {
    TL_RETURN_IF_NULL(m_shape);

    // check has shape leaved
    int i = m_touch_shapes.size();
    while (--i >= 0) {
        PhysicsShape* shape = m_touch_shapes[i];
        if (!CURRENT_CONTEXT.m_physics_scene->Overlap(*m_shape, *shape)) {
            OverlapResult result;
            result.m_dst_entity = shape->GetOwner();
            result.m_dst_shape = shape;
            TriggerLeaveEvent event{m_shape->GetOwner(), GetEventType(),
                                    result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
            m_touch_shapes.erase(m_touch_shapes.begin() + i);
        }
    }

    // send still touch shape
    if (m_trig_every_frame_when_touch) {
        for (auto shape : m_touch_shapes) {
            OverlapResult result;
            result.m_dst_entity = shape->GetOwner();
            result.m_dst_shape = shape;
            TriggerTouchEvent event{m_shape->GetOwner(), GetEventType(),
                                    result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
        }
    }

    // check entered shapes
    OverlapResult results[16];
    uint32_t count = CURRENT_CONTEXT.m_physics_scene->Overlap(
        *m_shape, results, std::size(results));
    for (size_t i = 0; i < count; i++) {
        auto& result = results[i];
        if (result.m_dst_entity == null_entity ||
            result.m_dst_shape == nullptr) {
            continue;
        }

        auto it = std::find(m_touch_shapes.begin(), m_touch_shapes.end(),
                            result.m_dst_shape);
        if (it == m_touch_shapes.end()) {
            m_touch_shapes.push_back(result.m_dst_shape);
            TriggerEnterEvent event{m_shape->GetOwner(), GetEventType(),
                                    result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
        }
    }
}

void TriggerComponentManager::Update() {
    PROFILE_SECTION();

    for (auto& [entity, trigger] : m_components) {
        TL_CONTINUE_IF_FALSE(trigger.m_enable && trigger.m_component->m_shape);

        auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
        TL_CONTINUE_IF_FALSE(transform);

        auto& global_mat = transform->GetGlobalMat();
        trigger.m_component->m_shape->MoveTo(GetPosition(global_mat));
    }

    for (auto& [entity, trigger] : m_components) {
        if (!trigger.m_enable || !trigger.m_component->m_shape) {
            continue;
        }

        trigger.m_component->Update();
    }
}
