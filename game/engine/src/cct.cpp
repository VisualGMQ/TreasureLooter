#include "engine/cct.hpp"

#include "engine/context.hpp"
#include "imgui.h"
#include "engine/imgui_id_generator.hpp"

CharacterController::CharacterController(Entity entity, const CCT& create_info)
    : m_skin{create_info.m_skin}, m_min_disp{create_info.m_min_disp} {
    m_actor = GAME_CONTEXT.m_physics_scene->CreateActor(
        entity, create_info.m_physics_actor);
}

CharacterController::~CharacterController() {
    GAME_CONTEXT.m_physics_scene->RemoveActor(m_actor);
}

bool CharacterController::EnableDebugOutput = false;

#define CCT_DEBUG_LOG(fmt, ...)                        \
    do {                                               \
        if (CharacterController::EnableDebugOutput) {  \
            LOGI("CCT Debug Log:" fmt, ##__VA_ARGS__); \
        }                                              \
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
    SweepResult hit;

    auto& physics_scene = GAME_CONTEXT.m_physics_scene;

    CCT_DEBUG_LOG("max iter = {}, begin iter", max_iter);
    CCT_DEBUG_LOG("start position: {}", m_actor->GetPosition());

    while (max_iter--) {
        if (disp_length <= m_min_disp) {
            CCT_DEBUG_LOG("disp length({}) < min disp, exit", disp_length);
            break;
        }

        if (disp.Dot(dir) <= 0) {
            CCT_DEBUG_LOG("move to opposite direction: {}, break", disp);
            break;
        }

        uint32_t hitted = physics_scene->Sweep(*m_actor, disp_normalized,
                                               disp_length + m_skin, &hit, 1);

        for (int i = 0; i < hitted; i++) {
            CCT_DEBUG_LOG("hitted {}: position = {}, normal = {},  t = {}", i,
                          hit.m_actor->GetPosition(), hit.m_normal,
                          hit.m_t);
        }

        if (!hitted) {
            CCT_DEBUG_LOG("not hitted, move along {}", disp);
            m_actor->Move(disp);
            break;
        }

        if (hit.m_is_initial_overlap) {
            CCT_DEBUG_LOG("initial overlap, move along {}", disp);
            m_actor->Move(disp);
            break;
        }

        CCT_DEBUG_LOG("hitted! hit flags: {}, normal: {}, t: {}",
                      hit.m_flags.Value(), hit.m_normal, hit.m_t);

        float actual_move_dist = 0;
        if (hit.m_t > m_skin) {
            actual_move_dist = hit.m_t - m_skin;
            m_actor->Move(actual_move_dist * disp_normalized);
            CCT_DEBUG_LOG("is less than skin({} < {}): {}, actual move dist {}",
                          hit.m_t, m_skin, hit.m_t < m_skin,
                          actual_move_dist);
            CCT_DEBUG_LOG("move to {}", m_actor->GetPosition());
        }

        disp_length -= actual_move_dist;

        CCT_DEBUG_LOG("remain disp length: {}", disp_length);

        auto [tangent, normal] =
            DecomposeVector(disp_normalized * disp_length, hit.m_normal);
        disp_length = tangent.Length();

        CCT_DEBUG_LOG("tangent: {}, length: {}", tangent, disp_length);

        disp = tangent;
        disp_normalized = disp / disp_length;
    }

    CCT_DEBUG_LOG("end iter, final position: {}", m_actor->GetPosition());

    m_actor->MoveTo(m_actor->GetPosition());
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