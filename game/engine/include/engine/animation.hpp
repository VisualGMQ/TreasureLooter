#pragma once
#include "engine/asset.hpp"
#include "engine/asset_manager_interface.hpp"
#include "engine/entity.hpp"
#include "engine/manager.hpp"
#include "engine/math.hpp"

#include "schema/anim.hpp"
#include "schema/bind_point_schema.hpp"
#include "engine/timer.hpp"

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
    virtual TimeType GetFinishTime() const = 0;
    virtual AnimationTrackType GetType() const = 0;
    virtual bool IsEmpty() const = 0;
};

template <typename T>
class IAnimationTrack : public AnimationTrackBase {
public:
    using keyframe_type = KeyFrame<T>;

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

    auto& GetKeyframes() const { return m_keyframes; }

    auto& GetKeyframes() { return m_keyframes; }

    bool IsEmpty() const override { return m_keyframes.empty(); }

protected:
    std::vector<keyframe_type> m_keyframes;
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
};

template <typename T>
class AnimationTrack<T, AnimationTrackType::Discrete>
    : public IAnimationTrack<T> {
public:
    AnimationTrackType GetType() const override {
        return AnimationTrackType::Discrete;
    }
};

class Animation {
public:
    void AddTrack(AnimationBindingPoint binding,
                  std::unique_ptr<AnimationTrackBase>&& track);
    void AddBindPointTrack(const std::string& name, std::unique_ptr<IAnimationTrack<Vec2>>&& track);

    void AddTracks(const SpriteRowColumnAnimationInfo& info);

    auto& GetTracks() const { return m_tracks; }

    auto& GetTracks() { return m_tracks; }

    auto& GetBindPointTracks() const { return m_bind_point_tracks; }

    auto& GetBindPointTracks() { return m_bind_point_tracks; }

private:
    std::unordered_map<AnimationBindingPoint,
                       std::unique_ptr<AnimationTrackBase>>
        m_tracks;
    std::unordered_map<std::string, std::unique_ptr<AnimationTrackBase>>
        m_bind_point_tracks;
};

using AnimationHandle = Handle<Animation>;

constexpr std::string_view Animation_AssetExtension = ".animation.xml";

using AnimationHandle = Handle<Animation>;

template <>
AssetLoadResult<Animation> LoadAsset<Animation>(const Path& filename);

void SaveAsset(const UUID& uuid, const Animation& payload,
               const Path& filename);

class AnimationManager : public AssetManagerBase<Animation> {
public:
    AnimationHandle Load(const Path& filename, bool force = false) override;

    AnimationHandle Create();
};
