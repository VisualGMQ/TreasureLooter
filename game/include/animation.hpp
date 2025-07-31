#pragma once
#include "asset.hpp"
#include "asset_manager.hpp"
#include "entity.hpp"
#include "log.hpp"
#include "manager.hpp"
#include "math.hpp"

#include "schema/animation.hpp"
#include "timer.hpp"

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

    virtual AnimationTrackType GetType() const = 0;
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

    void AddKeyframes(std::vector<keyframe_type>&& keyframes) {
        for (auto& keyframe : keyframes) {
            m_keyframes.emplace_back(std::move(keyframe));
        }
        keyframes.clear();
    }

    TimeType GetFinishTime() const override {
        if (m_keyframes.empty()) {
            return 0;
        }
        return m_keyframes.back().m_time;
    }

    void Rewind() override {
        m_cur_frame = -1;
        m_cur_time = 0;
    }

    auto& GetKeyframes() const { return m_keyframes; }

    auto& GetKeyframes() { return m_keyframes; }

    bool NeedSync() const {
        return m_cur_frame >= 0;
    }

protected:
    std::vector<keyframe_type> m_keyframes;
    TimeType m_cur_time{};
    int m_cur_frame = -1;
};

template <typename T, AnimationTrackType Type>
class AnimationTrack;

template <typename T>
class AnimationTrack<T, AnimationTrackType::Linear>
    : public IAnimationTrack<T> {
public:
    AnimationTrackType GetType() const override {
        return AnimationTrackType::Linear;
    }

    T GetValue() const override {
        if (this->m_keyframes.empty()) {
            return {};
        }

        if (this->m_cur_frame + 1 >= this->m_keyframes.size()) {
            return this->m_keyframes[this->m_cur_frame].m_value;
        }
        auto& cur_frame = this->m_keyframes[this->m_cur_frame];
        auto& next_frame = this->m_keyframes[this->m_cur_frame + 1];
        float t = (this->m_cur_time - cur_frame.m_time) /
                  (next_frame.m_time - cur_frame.m_time);
        return Lerp(cur_frame.m_value, next_frame.m_value, t);
    }
};

template <typename T>
class AnimationTrack<T, AnimationTrackType::Discrete>
    : public IAnimationTrack<T> {
public:
    AnimationTrackType GetType() const override {
        return AnimationTrackType::Discrete;
    }

    T GetValue() const override {
        if (this->m_keyframes.empty()) {
            return {};
        }
        return this->m_keyframes[this->m_cur_frame].m_value;
    }
};

class Animation {
public:
    static constexpr int InfLoop = -1;

    Animation() = default;
    Animation(const Path& filename);

    void AddTrack(AnimationBindingPoint binding,
                  std::unique_ptr<AnimationTrackBase>&& track) {
        m_tracks[binding] = std::move(track);
    }

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    void SetLoop(int);
    bool IsPlaying() const;

    void Update(TimeType delta_time);

    auto& GetTracks() const { return m_tracks; }

    auto& GetTracks() { return m_tracks; }

    void Sync(Entity);

    int GetLoopCount() const;

    TimeType GetCurTime() const;
    TimeType GetMaxTime() const;

    const Path& Filename() const;

private:
    bool m_is_playing = false;
    int m_loop{0};
    TimeType m_cur_time{};
    std::unordered_map<AnimationBindingPoint,
                       std::unique_ptr<AnimationTrackBase>>
        m_tracks;
    Path m_filename;
};

using AnimationHandle = Handle<Animation>;

class AnimationComponentManager : public ComponentManager<AnimationHandle> {
public:
    void Update(TimeType delta_time);
};

class AnimationManager : public AssetManagerBase<Animation> {
public:
    AnimationHandle Load(const Path& filename) override;

    AnimationHandle Create();
};

constexpr std::string_view Animation_AssetExtension = ".animation.xml";

template <>
AssetLoadResult<Animation> LoadAsset<Animation>(const Path& filename);

void SaveAsset(const UUID& uuid, const Animation& payload,
               const Path& filename);


using AnimationHandle = Handle<Animation>;