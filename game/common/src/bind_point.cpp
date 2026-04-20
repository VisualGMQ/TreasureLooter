#include "common/bind_point.hpp"

#include "common/context.hpp"
#include "common/debug_drawer.hpp"
#include "common/profile.hpp"

Vec2 BindPoint::GetGlobalPosition() const {
    return m_global_position;
}

void BindPoint::UpdateGlobalPosition(const Transform& parent) {
    Transform transform;
    transform.m_position = m_position;
    transform.UpdateMat(&parent);
    auto& global_mat = transform.GetGlobalMat();
    m_global_position = GetPosition(global_mat);
}

BindPoints::BindPoints(const std::vector<BindPointDefinition>& defs) {
    for (auto& def : defs) {
        BindPoint bind_point;
        bind_point.m_position = def.m_position;
        bind_point.m_name = def.m_name;
        m_bind_points[bind_point.m_name] = bind_point;
    }
}

void BindPointsComponentManager::Update() {
    PROFILE_SECTION();

    for (auto& [entity, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);

        Transform* transform = COMMON_CONTEXT.m_transform_manager->Get(entity);
        TL_CONTINUE_IF_FALSE(transform);

        for (auto& [_, bind_point] : component.m_component->m_bind_points) {
            bind_point.UpdateGlobalPosition(*transform);
        }
    }
}

void BindPointsComponentManager::ToggleDebugDraw() {
    m_should_debug_draw = !m_should_debug_draw;
}

void BindPointsComponentManager::RenderDebug(TimeType delta_time) const {
    PROFILE_SECTION();

    TL_RETURN_IF_FALSE(IsEnableDebugDraw());

    auto& debug_draw = COMMON_CONTEXT.m_debug_drawer;
    for (auto& [_, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);
        Rect rect;
        for (auto& [_, bind_point] : component.m_component->m_bind_points) {
            rect.m_center = bind_point.GetGlobalPosition();
            rect.m_half_size = {2, 2};
            debug_draw->FillRect(rect, Color::Green, IDebugDrawer::kOneFrame, true);
        }
    }
}

bool BindPointsComponentManager::IsEnableDebugDraw() const {
    return m_should_debug_draw;
}
