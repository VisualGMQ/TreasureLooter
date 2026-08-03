#pragma once
#include "common/animation.hpp"
#include "common/animation_player.hpp"

#include <functional>

class Tween {
public:
    enum class State {
        Idle,
        Playing,
        Paused,
        Killed,
        Finished,
    };

    template <typename T>
    Tween& AddProperty(T& payload, const T& dst, TimeType duration,
                       int loop = 0) {
        if (m_parallel_mode) {
            addToParallelPhase(payload, dst, duration, loop);
        } else {
            addToNormalPhase(payload, dst, duration, loop);
        }
        return *this;
    }

    Tween& BeginParallel() {
        m_parallel_mode = true;
        m_parallel_phases.emplace_back();
        m_parallel_phase_index_added = false;
        return *this;
    }

    Tween& EndParallel() {
        m_parallel_mode = false;
        m_parallel_phase_index_added = false;
        return *this;
    }

    void Play();
    void Pause();
    void Kill();

    [[nodiscard]] bool IsFinish() const;

private:
    friend class TweenManager;

    static constexpr int InfiniteLoop = -1;

    struct Phase {
        std::unique_ptr<AnimationTrackBase> m_track;
        std::unique_ptr<AnimationTrackPlayerBase> m_track_player;
        std::function<void()> m_sync_callback;
        int m_loop{};
        int m_loop_count{};
        bool m_finished{false};
    };

    struct ParallelPhase {
        std::vector<Phase> m_phases;
    };

    enum class PhaseType {
        Normal,
        Parallel,
    };

    class PhaseIndex {
    public:
        PhaseIndex(PhaseType type, uint16_t index) {
            m_index = index | ((type == PhaseType::Parallel) << 15);
        }

        [[nodiscard]] bool IsNormalPhase() const {
            return !(m_index & (1 << 15));
        }

        [[nodiscard]] bool IsParallelPhase() const {
            return m_index & (1 << 15);
        }

        [[nodiscard]] uint16_t GetIndex() const { return m_index & 0x7FFF; }

    private:
        uint16_t m_index{};
    };

    void Update(TimeType dt);
    static void UpdateSinglePhase(Phase& phase, TimeType dt);

    template <typename T>
    Phase createPhase(T& payload, const T& dst, TimeType duration, int loop) {
        using track_type = AnimationTrack<T, AnimationTrackType::Linear>;
        using player_type = AnimationTrackPlayer<T, AnimationTrackType::Linear>;

        auto track = std::make_unique<track_type>();
        track->AddKeyframe({payload, 0});
        track->AddKeyframe({dst, duration});

        Phase phase;
        phase.m_track = std::move(track);

        auto* track_ptr = static_cast<track_type*>(phase.m_track.get());
        auto player = std::make_unique<player_type>(*track_ptr);

        auto* player_ptr = player.get();
        phase.m_sync_callback = [&payload, player_ptr]() {
            if (player_ptr->NeedSync()) {
                payload = player_ptr->GetValue();
            }
        };

        phase.m_track_player = std::move(player);
        phase.m_loop = loop;

        return phase;
    }

    template <typename T>
    void addToNormalPhase(T& payload, const T& dst, TimeType duration,
                          int loop) {
        m_phases.emplace_back(createPhase(payload, dst, duration, loop));
        m_phase_indices.emplace_back(PhaseType::Normal, m_phases.size() - 1);
    }

    template <typename T>
    void addToParallelPhase(T& payload, const T& dst, TimeType duration,
                            int loop) {
        TL_ASSERT(!m_parallel_phases.empty());

        auto& parallel = m_parallel_phases.back();
        parallel.m_phases.emplace_back(createPhase(payload, dst, duration, loop));

        if (!m_parallel_phase_index_added) {
            m_phase_indices.emplace_back(PhaseType::Parallel,
                                         m_parallel_phases.size() - 1);
            m_parallel_phase_index_added = true;
        }
    }

    State m_state{State::Idle};
    int m_current_phase_idx{0};

    std::vector<Phase> m_phases;
    std::vector<ParallelPhase> m_parallel_phases;
    std::vector<PhaseIndex> m_phase_indices;
    bool m_parallel_mode{false};
    bool m_parallel_phase_index_added{false};
};

class TweenManager {
public:
    Tween& Create();
    void Remove(Tween&);
    void Clear();
    void Update(TimeType dt);

private:
    std::vector<Tween> m_tweens;
};
