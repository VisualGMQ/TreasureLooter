#include "engine/trigger.hpp"

#include "engine/context.hpp"
#include "engine/profile.hpp"

TriggerEnterEvent::TriggerEnterEvent(TriggerEventType type,
                                     OverlapResult overlap)
    : m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerEnterEvent::GetType() const {
    return m_type;
}

TriggerTouchEvent::TriggerTouchEvent(TriggerEventType type,
                                     OverlapResult overlap)
    : m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerLeaveEvent::GetType() const {
    return m_type;
}

TriggerLeaveEvent::TriggerLeaveEvent(TriggerEventType type,
                                     OverlapResult overlap)
    : m_type{type}, m_overlap{overlap} {}

TriggerEventType TriggerTouchEvent::GetType() const {
    return m_type;
}

Trigger::Trigger(Entity entity, const TriggerDefinition& info)
    : m_event_type{info.m_event_type},
      m_trig_every_frame_when_touch{info.m_trig_every_frame_when_touch} {
    m_actor =
        CURRENT_CONTEXT.m_physics_scene->CreateActor(entity, info.m_physics_actor);
}

Trigger::Trigger(Entity entity, const Rect& rect, TriggerEventType event_type,
                 const CollisionGroup& collision_layer,
                 const CollisionGroup& collision_mask)
    : m_event_type{event_type} {
    m_actor = CURRENT_CONTEXT.m_physics_scene->CreateActor(entity, rect);
    if (m_actor) {
        m_actor->SetCollisionLayer(collision_layer);
        m_actor->SetCollisionMask(collision_mask);
    }
}

Trigger::Trigger(Entity entity, const Circle& circle,
                 TriggerEventType event_type,
                 const CollisionGroup& collision_layer,
                 const CollisionGroup& collision_mask)
    : m_event_type{event_type} {
    m_actor = CURRENT_CONTEXT.m_physics_scene->CreateActor(entity, circle);
    if (m_actor) {
        m_actor->SetCollisionLayer(collision_layer);
        m_actor->SetCollisionMask(collision_mask);
    }
}

const PhysicsActor* Trigger::GetActor() const {
    return m_actor;
}

PhysicsActor* Trigger::GetActor() {
    return m_actor;
}

Trigger::~Trigger() {
    CURRENT_CONTEXT.m_physics_scene->RemoveActor(m_actor);
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
    // check has actor leaved
    int i = m_touch_actors.size();
    while (--i >= 0) {
        PhysicsActor* actor = m_touch_actors[i];
        if (!CURRENT_CONTEXT.m_physics_scene->Overlap(*m_actor, *actor)) {
            OverlapResult result;
            result.m_dst_entity = actor->GetEntity();
            result.m_dst_actor = actor;
            TriggerLeaveEvent event{GetEventType(), result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
            m_touch_actors.erase(m_touch_actors.begin() + i);
        }
    }

    // send still touch actors
    if (m_trig_every_frame_when_touch) {
        for (auto actor : m_touch_actors) {
            OverlapResult result;
            result.m_dst_entity = actor->GetEntity();
            result.m_dst_actor = actor;
            TriggerTouchEvent event{GetEventType(), result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
        }
    }

    // check entered actors
    OverlapResult results[16];
    uint32_t count = CURRENT_CONTEXT.m_physics_scene->Overlap(*m_actor, results,
                                                           std::size(results));
    for (size_t i = 0; i < count; i++) {
        auto& result = results[i];
        if (result.m_dst_entity == null_entity ||
            result.m_dst_actor == nullptr) {
            continue;
        }

        auto it = std::find(m_touch_actors.begin(), m_touch_actors.end(),
                            result.m_dst_actor);
        if (it == m_touch_actors.end()) {
            m_touch_actors.push_back(result.m_dst_actor);
            TriggerEnterEvent event{GetEventType(), result};
            CURRENT_CONTEXT.m_event_system->EnqueueEvent(event);
        }
    }
}

void TriggerComponentManager::Update() {
    PROFILE_PHYSICS_SECTION(__FUNCTION__);
    
    for (auto& [entity, trigger] : m_components) {
        if (!trigger.m_enable || !trigger.m_component->m_actor) {
            continue;
        }

        auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
        if (!transform) {
            continue;
        }

        trigger.m_component->m_actor->MoveTo(transform->m_position);
    }

    for (auto& [entity, trigger] : m_components) {
        if (!trigger.m_enable || !trigger.m_component->m_actor) {
            continue;
        }

        trigger.m_component->Update();
    }
}
