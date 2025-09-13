#include "engine/enemy_state.hpp"

#include <random>

#include "engine/context.hpp"

void EnemyIdleState::OnEnter(EnemyMotorContext* state) {
    auto sprite = state->GetSprite();
    switch (state->GetDirection()) {
        case CharacterDirection::Left:
            sprite->m_region.m_topleft = {16 * 3, 0};
            break;
        case CharacterDirection::Right:
            sprite->m_region.m_topleft = {16 * 3, 0};
            break;
        case CharacterDirection::Down:
            sprite->m_region.m_topleft = {16 * 0, 0};
            break;
        case CharacterDirection::Up:
            sprite->m_region.m_topleft = {16 * 1, 0};
            break;
    }
}

void EnemyIdleState::OnUpdate(EnemyMotorContext* state) {
    state->GetAnimationPlayer()->Stop();
    if (state->m_cur_idle_time < state->m_idle_time) {
        state->m_cur_idle_time += CURRENT_CONTEXT.m_time->GetElapseTime();
    } else {
        state->m_cur_idle_time = 0;
        auto& state_machine = state->GetStateMachine();
        state_machine.ChangeState(
            &StateSingletonManager::GetState<EnemyMoveState>());
    }
}

void EnemyMoveState::OnEnter(EnemyMotorContext* state) {
    constexpr float dist = 30;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distr(-1, 1);
    Vec2 dir = {distr(gen), distr(gen)};
    dir = dir.Normalize();
    state->m_target_position = state->GetPosition() + dir * dist;
}

void EnemyMoveState::OnQuit(EnemyMotorContext* state) {
    state->m_cur_force_idle_time = 0;
    state->m_target_position = state->GetPosition();
}

void EnemyMoveState::OnUpdate(EnemyMotorContext* state) {
    state->m_cur_force_idle_time += CURRENT_CONTEXT.m_time->GetElapseTime();
    auto position = state->GetPosition();
    auto disp = position - state->m_target_position;
    if (disp.LengthSquared() <= 0.00001 || state->m_cur_force_idle_time >= state
        ->m_force_idle_time) {
        auto& state_machine = state->GetStateMachine();
        state_machine.ChangeState(
            &StateSingletonManager::GetState<EnemyIdleState>());
    } else {
        auto dir = disp.Normalize();
        state->Move(dir, CURRENT_CONTEXT.m_time->GetElapseTime());
    }
}
