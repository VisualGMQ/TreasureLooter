#pragma once
#include "entity.hpp"
#include "log.hpp"
#include "manager.hpp"
#include "math.hpp"

#include "timer.hpp"

#include <array>
#include <memory>
#include <vector>

template <typename T>
struct KeyFrame {
    T m_value{};
    TimeType m_time{};
};

class AnimationTrackBase {
public:
    virtual ~AnimationTrackBase() = default;
    virtual void Update(TimeType) = 0;
    virtual void Rewind() = 0;
    virtual TimeType GetFinishTime() const = 0;
};

enum class AnimationTrackType {
    Linear,
    Discrete,
};

template <typename T>
class IAnimationTrack : public AnimationTrackBase {
public:
    using keyframe_type = KeyFrame<T>;

    void Update(TimeType delta_time) override {
        if (m_cur_frame + 1 >= m_keyframes.size()) {
            return;
        }

        while (m_cur_frame + 1 < m_keyframes.size()) {
            auto& cur_frame = m_keyframes[m_cur_frame];
            auto& next_frame = m_keyframes[m_cur_frame + 1];

            m_cur_time += delta_time;
            if (next_frame.m_time <= m_cur_time) {
                m_cur_frame++;
            } else {
                break;
            }
        }

        m_cur_time = std::min(m_keyframes.back().m_time, m_cur_time);
    }

    virtual T GetValue() const = 0;

    // key frames must insert by time
    void AddKeyframe(const keyframe_type& keyframe) {
        m_keyframes.emplace_back(keyframe);
    }

    TimeType GetFinishTime() const override {
        if (m_keyframes.empty()) {
            return 0;
        }
        return m_keyframes.back().m_time;
    }

    void Rewind() override {
        m_cur_frame = 0;
        m_cur_time = 0;
    }

protected:
    std::vector<keyframe_type> m_keyframes;
    TimeType m_cur_time{};
    size_t m_cur_frame{};
};

template <typename T, AnimationTrackType Type>
class AnimationTrack;

template <typename T>
class AnimationTrack<T, AnimationTrackType::Linear>
    : public IAnimationTrack<T> {
public:
    T GetValue() const override {
        if (m_cur_frame + 1 >= m_keyframes.size()) {
            return m_keyframes[m_cur_frame].m_value;
        }
        auto& cur_frame = m_keyframes[m_cur_frame];
        auto& next_frame = m_keyframes[m_cur_frame + 1];
        float t = (m_cur_time - cur_frame.m_time) /
                  (next_frame.m_time - cur_frame.m_time);
        return Lerp(cur_frame.m_value, next_frame.m_value, t);
    }
};

template <typename T>
class AnimationTrack<T, AnimationTrackType::Discrete>
    : public IAnimationTrack<T> {
public:
    T GetValue() const override { return m_keyframes[m_cur_frame].m_value; }
};

enum class AnimationBindingPoint {
    TransformPositionX = 0,
    TransformPositionY,
    TransformScaleX,
    TransformScaleY,
    TransformRotation,

    SpriteRegion,
    SpriteSizeX,
    SpriteSizeY,

    SpriteImage,

    BindingNum,
};

class Animation {
public:
    static constexpr int InfLoop = -1;

    void AddTrack(AnimationBindingPoint binding,
                  std::unique_ptr<AnimationTrackBase> track) {
        m_max_time = std::max(m_max_time, track->GetFinishTime());
        m_tracks[static_cast<size_t>(binding)] = std::move(track);
    }

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    void SetLoop(int);

    void Update(TimeType delta_time);

    auto& GetTracks() const { return m_tracks; }

    void Sync(Entity);

private:
    bool m_is_playing = false;
    int m_loop{0};
    TimeType m_max_time{};
    TimeType m_cur_time{};
    std::array<std::unique_ptr<AnimationTrackBase>,
               static_cast<size_t>(AnimationBindingPoint::BindingNum)>
        m_tracks;
};

class AnimationManager: public ComponentManager<Animation> {
public:
    void Update(TimeType delta_time);
};