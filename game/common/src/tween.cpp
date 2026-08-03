#include "common/tween.hpp"

void Tween::Play() {
    m_state = State::Playing;
    m_current_phase_idx = 0;

    for (auto& phase : m_phases) {
        phase.m_track_player->Rewind();
        phase.m_finished = false;
        phase.m_loop_count = 0;
    }
    for (auto& parallel : m_parallel_phases) {
        for (auto& phase : parallel.m_phases) {
            phase.m_track_player->Rewind();
            phase.m_finished = false;
            phase.m_loop_count = 0;
        }
    }
}

void Tween::Pause() {
    if (m_state == State::Playing) {
        m_state = State::Paused;
    }
}

void Tween::Kill() {
    m_state = State::Killed;
}

bool Tween::IsFinish() const {
    return m_state == State::Finished || m_state == State::Killed;
}

void Tween::UpdateSinglePhase(Phase& phase, TimeType dt) {
    if (phase.m_finished) {
        return;
    }

    auto finish_time = phase.m_track_player->GetFinishTime();
    phase.m_track_player->Update(dt);
    phase.m_sync_callback();

    if (!phase.m_track_player->IsComplete()) {
        return;
    }

    if (phase.m_loop == InfiniteLoop ||
        phase.m_loop_count < phase.m_loop) {
        phase.m_track_player->Rewind();
        phase.m_loop_count++;

        TimeType overflow = finish_time > 0 ? dt : 0;
        if (overflow > 0) {
            phase.m_track_player->Update(overflow);
            phase.m_sync_callback();
        }
    } else {
        phase.m_finished = true;
    }
}

void Tween::Update(TimeType dt) {
    if (m_state != State::Playing) {
        return;
    }

    if (m_phase_indices.empty() ||
        m_current_phase_idx >= static_cast<int>(m_phase_indices.size())) {
        m_state = State::Finished;
        return;
    }

    auto& pi = m_phase_indices[m_current_phase_idx];

    if (pi.IsNormalPhase()) {
        auto& phase = m_phases[pi.GetIndex()];
        UpdateSinglePhase(phase, dt);

        if (phase.m_finished) {
            m_current_phase_idx++;
            if (m_current_phase_idx >=
                static_cast<int>(m_phase_indices.size())) {
                m_state = State::Finished;
            }
        }
    } else {
        auto& parallel = m_parallel_phases[pi.GetIndex()];
        bool all_finished = true;

        for (auto& phase : parallel.m_phases) {
            UpdateSinglePhase(phase, dt);
            if (!phase.m_finished) {
                all_finished = false;
            }
        }

        if (all_finished) {
            m_current_phase_idx++;
            if (m_current_phase_idx >=
                static_cast<int>(m_phase_indices.size())) {
                m_state = State::Finished;
            }
        }
    }
}

Tween& TweenManager::Create() {
    return m_tweens.emplace_back();
}

void TweenManager::Remove(Tween& tween) {
    tween.Kill();
}

void TweenManager::Clear() {
    m_tweens.clear();
}

void TweenManager::Update(TimeType dt) {
    for (auto& tween : m_tweens) {
        tween.Update(dt);
    }

    size_t i = m_tweens.size();
    while (i > 0) {
        i--;
        if (m_tweens[i].IsFinish()) {
            if (i != m_tweens.size() - 1) {
                m_tweens[i] = std::move(m_tweens.back());
            }
            m_tweens.pop_back();
        }
    }
}
