#include "cct.hpp"

#include "context.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"
#include "instance_display.hpp"
#include "schema/display/common.hpp"

CharacterController::CharacterController(Entity entity, const CCT& create_info)
    : m_skin{create_info.m_skin}, m_min_disp{create_info.m_min_disp} {
    m_actor = GAME_CONTEXT.m_physics_scene->CreateActor(
        entity, create_info.m_physics_actor);
}

CharacterController::~CharacterController() {
    GAME_CONTEXT.m_physics_scene->RemoveActor(m_actor);
}

bool CharacterController::EnableDebugOutput = false;

#define CCT_DEBUG_LOG(fmt, ...)                            \
    do {                                              \
        if (CharacterController::EnableDebugOutput) { \
            LOGI("CCT Debug Log:" fmt, ##__VA_ARGS__);   \
        }                                             \
    } while (0)

void CharacterController::MoveAndSlide(const Vec2& dir) {
    if (!m_actor) {
        CCT_DEBUG_LOG("actor is nullptr");
        return;
    }

    float disp_length = dir.Length();
    CCT_DEBUG_LOG("disp: {}", dir);
    CCT_DEBUG_LOG("disp length: {}", disp_length);
    if (disp_length <= m_min_disp) {
        CCT_DEBUG_LOG("disp length too small, exit");
        return;
    }

    Vec2 disp = dir;
    Vec2 disp_normalized = disp / disp_length;
    uint32_t max_iter = MaxIter;
    HitResult hit;

    auto& physics_scene = GAME_CONTEXT.m_physics_scene;
    Vec2 position = GetPosition();

    CCT_DEBUG_LOG("max iter = {}, begin iter", max_iter);
    CCT_DEBUG_LOG("start position: {}", position);

    while (max_iter--) {
        bool hitted = physics_scene->Sweep(*m_actor, disp_normalized,
                                           disp_length, &hit, 1);

        if (!hitted) {
            CCT_DEBUG_LOG("not hitted, move along {}", disp);
            position += disp;
            break;
        }

        CCT_DEBUG_LOG("hitted! hit flags: {}, normal: {}, t: {}", hit.m_flags.Value(),
                      hit.m_normal, hit.m_t);

        float actual_move_dist =
            hit.m_t < m_skin ? -(m_skin - hit.m_t) : hit.m_t - m_skin;

        CCT_DEBUG_LOG("is less than skin({} < {}): {}, actual move dist {}",
                      hit.m_t, m_skin, hit.m_t < m_skin, actual_move_dist);

        position += actual_move_dist * disp_normalized;

        CCT_DEBUG_LOG("move to {}", position);

        disp_length -= actual_move_dist;

        CCT_DEBUG_LOG("remain disp length: {}", disp_length);

        if (disp_length <= m_min_disp) {
            CCT_DEBUG_LOG("disp length < min disp, exit");
            break;
        }

        auto [tangent, normal] =
            DecomposeVector(disp_normalized * disp_length, hit.m_normal);
        disp_length = tangent.Length();

        CCT_DEBUG_LOG("tangent: {}, length: {}", tangent, disp_length);

        if (disp_length <= std::numeric_limits<float>::epsilon()) {
            CCT_DEBUG_LOG("disp too small, exit");
            break;
        }

        disp = tangent;
        disp_normalized = disp / disp_length;
    }

    CCT_DEBUG_LOG("end iter, final position: {}", position);
    m_actor->MoveTo(position);
}

Vec2 CharacterController::GetPosition() const {
    if (m_actor) {
        return m_actor->GetPosition();
    }
    return {};
}

void CharacterController::SetSkin(float skin) {
    m_skin = skin;
}

void CharacterController::SetMinDisp(float disp) {
    m_min_disp = disp;
}

void CharacterController::Teleport(const Vec2& pos) {
    if (m_actor) {
        m_actor->MoveTo(pos);
    }
}

const PhysicsActor* CharacterController::GetActor() const {
    return m_actor;
}