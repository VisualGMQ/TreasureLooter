#include "cct.hpp"

#include "context.hpp"

CharacterController::CharacterController(const CCT& create_info)
    : m_circle{{}, create_info.m_radius},
      m_skin{create_info.m_skin},
      m_min_disp{create_info.m_min_disp} {}

void CharacterController::MoveAndSlide(const Vec2& dir) {
    auto& physics_scene = GAME_CONTEXT.m_physics_scene;

    float disp_length = dir.Length();
    if (disp_length <= std::numeric_limits<float>::epsilon()) {
        return;
    }

    Vec2 disp = dir;
    Vec2 disp_normalized = disp / disp_length;
    uint32_t max_iter = MaxIter;
    HitResult hit;
    while (max_iter--) {
        PhysicsActor actor{m_circle, PhysicsActor::StorageType::Normal};
        bool hitted = physics_scene->Sweep(actor, disp_normalized,
                                                   disp_length, &hit, 1);

        if (!hitted) {
            m_circle.m_center += disp;
            return;
        }

        float actual_move_dist = hit.m_t < m_skin ? -(m_skin - hit.m_t) : hit.m_t - m_skin;

        m_circle.m_center +=
            actual_move_dist * disp_normalized;

        disp_length -= actual_move_dist;

        if (disp_length <= m_min_disp) {
            return;
        }

        auto [tangent, normal] =
            DecomposeVector(disp_normalized * disp_length, hit.m_normal);
        disp_length = tangent.Length();
        if (disp_length <= std::numeric_limits<float>::epsilon()) {
            return;
        }
        disp = tangent;
        disp_normalized = disp / disp_length;
    }
}

const Vec2& CharacterController::GetPosition() const {
    return m_circle.m_center;
}

void CharacterController::SetSkin(float skin) {
    m_skin = skin;
}

void CharacterController::SetMinDisp(float disp) {
    m_min_disp = disp;
}

void CharacterController::Teleport(const Vec2& pos) {
    m_circle.m_center = pos;
}

const Circle& CharacterController::GetCircle() const {
    return m_circle;
}

bool CCTManager::IsEnableDebugDraw() const {
    return m_enable_debug_draw;
}

void CCTManager::ToggleDebugDraw() {
    m_enable_debug_draw = !m_enable_debug_draw;
}

void CCTManager::RenderDebug() {
    if (!IsEnableDebugDraw()) {
        return;
    }
    for (auto& [_, cct] : m_components) {
        if (!cct.m_enable) {
            continue;
        }
        GAME_CONTEXT.m_renderer->DrawCircle(cct.m_component->GetCircle(),
                                            Color::Green);
    }
}