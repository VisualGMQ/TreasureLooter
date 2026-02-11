#pragma once
#include "engine/animation.hpp"
#include "schema/anim_player.hpp"

class AnimationTrackPlayerBase {
public:
    virtual ~AnimationTrackPlayerBase() = default;
    virtual void Update(TimeType) = 0;
    virtual void Rewind() = 0;
    virtual AnimationTrackType GetType() const = 0;
    virtual TimeType GetFinishTime() const = 0;
};

template <typename T>
class IAnimationTrackPlayer : public AnimationTrackPlayerBase {
public:
    using keyframe_type = KeyFrame<T>;

    explicit IAnimationTrackPlayer(const IAnimationTrack<T>& track)
        : m_track(track) {}

    void Rewind() override {
        m_cur_frame = -1;
        m_cur_time = 0;
    }

    void Update(TimeType delta_time) override {
        auto& keyframes = m_track.GetKeyframes();
        if (m_cur_frame + 1 >= keyframes.size()) {
            return;
        }

        while (m_cur_frame + 1 < keyframes.size()) {
            auto& next_frame = keyframes[m_cur_frame + 1];

            m_cur_time += delta_time;
            if (next_frame.m_time <= m_cur_time) {
                m_cur_frame++;
            } else {
                break;
            }
        }

        m_cur_time = std::min(keyframes.back().m_time, m_cur_time);
    }

    virtual T GetValue() const = 0;

    bool NeedSync() const { return m_cur_frame >= 0; }

    auto& GetTrack() const { return m_track; }

    AnimationTrackType GetType() const override { return m_track.GetType(); }

    TimeType GetFinishTime() const override { return m_track.GetFinishTime(); }

protected:
    const IAnimationTrack<T>& m_track;
    TimeType m_cur_time{};
    int m_cur_frame = -1;
};

template <typename T, AnimationTrackType Type>
class AnimationTrackPlayer;

template <typename T>
class AnimationTrackPlayer<T, AnimationTrackType::Linear>
    : public IAnimationTrackPlayer<T> {
public:
    using track_type = AnimationTrack<T, AnimationTrackType::Linear>;

    explicit AnimationTrackPlayer(const track_type& track)
        : IAnimationTrackPlayer<T>(track) {}

    T GetValue() const override {
        auto& track = static_cast<const track_type&>(this->GetTrack());
        auto& keyframes = track.GetKeyframes();

        if (keyframes.empty()) {
            return {};
        }

        if (this->m_cur_frame + 1 >= keyframes.size()) {
            return keyframes[this->m_cur_frame].m_value;
        }
        auto& cur_frame = keyframes[this->m_cur_frame];
        auto& next_frame = keyframes[this->m_cur_frame + 1];
        float t = (this->m_cur_time - cur_frame.m_time) /
                  (next_frame.m_time - cur_frame.m_time);
        return Lerp(cur_frame.m_value, next_frame.m_value, t);
    }
};

template <typename T>
class AnimationTrackPlayer<T, AnimationTrackType::Discrete>
    : public IAnimationTrackPlayer<T> {
public:
    using track_type = AnimationTrack<T, AnimationTrackType::Discrete>;

    using IAnimationTrackPlayer<T>::IAnimationTrackPlayer;

    T GetValue() const override {
        auto& track = static_cast<const track_type&>(this->GetTrack());
        auto& keyframes = track.GetKeyframes();

        if (keyframes.empty()) {
            return {};
        }
        return keyframes[this->m_cur_frame].m_value;
    }
};

class AnimationPlayer {
public:
    static constexpr int InfLoop = -1;

    AnimationPlayer() = default;
    explicit AnimationPlayer(const AnimationPlayerDefinition&);

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    void SetLoop(int);
    bool IsPlaying() const;

    int GetLoopCount() const;
    TimeType GetCurTime() const;
    TimeType GetMaxTime() const;

    void ChangeAnimation(const Path& filename);
    void ChangeAnimation(AnimationHandle);
    void ChangeAnimation(UUID);
    void ClearAnimation();
    bool HasAnimation() const;

    void Update(TimeType delta_time);
    void Sync(Entity entity);

    void SetRate(float rate);
    float GetRate() const;

    AnimationHandle GetAnimation() const;

    void EnableAutoPlay(bool enable);
    bool IsAutoPlayEnabled() const;

private:
    AnimationHandle m_animation;

    std::unordered_map<AnimationBindingPoint,
                       std::unique_ptr<AnimationTrackPlayerBase>>
        m_track_players;
    std::unordered_map<std::string, std::unique_ptr<AnimationTrackPlayerBase>>
        m_bind_point_track_players;

    bool m_auto_play = false;
    bool m_is_playing = false;
    int m_loop{0};
    TimeType m_cur_time{};
    float m_rate = 1.0;
};

class AnimationPlayerManager : public ComponentManager<AnimationPlayer> {
public:
    void Update(TimeType delta_time);
};
