#include "gizmo.hpp"
#include "engine/context.hpp"
#include "engine/physics.hpp"

GizmoID Gizmo::GetID() const {
    return m_id;
}

const Vec2& Gizmo::GetPosition() const {
    return m_position;
}

void Gizmo::MoveTo(const Vec2& position) {
    m_position = position;
    m_moved = true;
}

void Gizmo::SetPosition(const Vec2& position) {
    m_position = position;
    m_moved = false;
}

bool Gizmo::fetchPosition(Vec2& position) {
    position = m_position;
    bool moved = m_moved;
    m_moved = false;
    return moved;
}

void Gizmo::Show() {
    m_visiable = true;
}

void Gizmo::Hide() {
    m_visiable = false;
}

bool Gizmo::IsVisiable() const {
    return m_visiable;
}

void Gizmo::Hovering(bool is_hovering) {
    m_hovering = is_hovering;
}

bool Gizmo::IsHovering() const {
    return m_hovering;
}

AxisGizmo::AxisGizmo(GizmoID id) : Gizmo{id} {}

void AxisGizmo::SetLen(float len) {
    m_len = len;
}

void AxisGizmo::SetWidth(float width) {
    m_width = width;
}

bool AxisGizmo::IsPointIn(const Vec2& q) const {
    Vec2 position = GetPosition();
    Vec2 x_axis_nearest_pt = NearestCapsulePoint(q, m_width + 10, position,
                                                 position + Vec2{m_len, 0});
    Vec2 y_axis_nearest_pt = NearestCapsulePoint(q, m_width + 10, position,
                                                 position + Vec2{0, m_len});

    return FLT_EQ(x_axis_nearest_pt.DistTo(q), 0) ||
           FLT_EQ(y_axis_nearest_pt.DistTo(q), 0);
}

void AxisGizmo::Draw(CommonContext& ctx) {
    Vec2 position = GetPosition();
    Rect x_axis_rect, y_axis_rect;
    x_axis_rect.m_half_size.w = m_len;
    x_axis_rect.m_half_size.h = m_width;
    x_axis_rect.m_center = position + Vec2{x_axis_rect.m_half_size.w, 0} * 0.5;

    y_axis_rect.m_half_size.w = m_width;
    y_axis_rect.m_half_size.h = m_len;
    y_axis_rect.m_center = position + Vec2{0, y_axis_rect.m_half_size.h} * 0.5;

    x_axis_rect.m_center = (x_axis_rect.m_center - ctx.m_camera.GetPosition()) * ctx.m_camera.GetScale();
    CURRENT_CONTEXT.m_camera.transform(&x_axis_rect.m_center, nullptr);
    CURRENT_CONTEXT.m_camera.transform(&y_axis_rect.m_center, nullptr);

    ctx.m_renderer->FillRect(x_axis_rect,
                             IsHovering() ? Color{1, 0, 0, 0.5} : Color::Red);
    ctx.m_renderer->FillRect(x_axis_rect,
                             IsHovering() ? Color{0, 1, 0, 0.5} : Color::Green);
}

AxisGizmo& GizmoManager::CreateAxisGizmo() {
    GizmoID id = genID();
    auto result = m_gizmoes.emplace(id, std::make_unique<AxisGizmo>(id));
    return static_cast<AxisGizmo&>(*result.first->second);
}

ButtonGizmo& GizmoManager::CreateButtonGizmo() {
    GizmoID id = genID();
    auto result = m_gizmoes.emplace(id, std::make_unique<ButtonGizmo>(id));
    return static_cast<ButtonGizmo&>(*result.first->second);
}

ButtonGizmo::ButtonGizmo(GizmoID id) : Gizmo{id} {}

void ButtonGizmo::SetLen(float len) {
    m_len = len;
}

bool ButtonGizmo::IsPointIn(const Vec2& p) const {
    Rect rect;
    rect.m_center = GetPosition();
    rect.m_half_size = Vec2{m_len, m_len};

    return IsPointInRect(p, rect);
}

void ButtonGizmo::Draw(CommonContext& ctx) {
    Rect rect;
    rect.m_center = GetPosition();
    rect.m_half_size = Vec2{m_len, m_len};
    CURRENT_CONTEXT.m_camera.transform(&rect.m_center, nullptr);
    ctx.m_renderer->FillRect(rect,
                             IsHovering() ? Color{0, 1, 0, 0.5} : Color::Green);
}

GizmoID GizmoManager::genID() {
    return m_cur_id++;
}

void GizmoManager::Update(CommonContext& ctx) {
    auto& mouse = ctx.m_mouse;
    auto& left_btn = mouse->Get(MouseButtonType::Left);
    auto mouse_position = mouse->Position();

    if (m_cur_id != InvalidGizmoID && left_btn.IsPressing()) {
        if (auto it = m_gizmoes.find(m_cur_id); it != m_gizmoes.end()) {
            it->second->MoveTo(mouse_position);
        }
    }

    if (left_btn.IsRelease() && m_cur_id != InvalidGizmoID) {
        if (auto it = m_gizmoes.find(m_cur_id); it != m_gizmoes.end()) {
            it->second->Hovering(false);
            m_cur_id = InvalidGizmoID;
        }
    }

    if (m_cur_id == InvalidGizmoID && left_btn.IsPressed()) {
        for (auto&& [id, gizmo] : m_gizmoes) {
            if (!gizmo->IsVisiable()) {
                continue;
            }
            if (gizmo->IsPointIn(mouse_position)) {
                gizmo->Hovering(true);
                m_cur_id = id;
                return;
            }
        }
    }
}

void GizmoManager::Draw(CommonContext& ctx) {
    for (auto&& [id, gizmo] : m_gizmoes) {
        if (!gizmo->IsVisiable()) {
            continue;
        }
        gizmo->Draw(ctx);
    }
}

void GizmoManager::Remove(GizmoID id) {
    m_gizmoes.erase(id);
    if (m_cur_id == id) {
        m_cur_id = InvalidGizmoID;
    }
}
