#pragma once
#include "macro.hpp"
#include "math.hpp"
#include "pch.hpp"
#include "texture.hpp"
#include "timer.hpp"


namespace tl {

enum class Interpolate {
    None,
    Linear,
};

template <typename T, bool CanInterpolate>
struct Keyframe {
    Keyframe(T value, uint32_t time,
             Interpolate interpolate = Interpolate::Linear)
        : value{value}, time{time}, interpolate{interpolate} {}

    T value;
    TimeType time;
    Interpolate interpolate = Interpolate::Linear;
};

template <typename T, bool CanInterp>
struct Track {
    std::vector<Keyframe<T, CanInterp>> keyframes;
};

template <typename T, bool CanInterp>
struct TrackPlayInfo {
    T curData{};
    uint32_t curFrame = 0;
};

enum class Vec2BindPoint {
    GOPosition = 0,
    GOScale,

    _Count,
};

enum class FloatBindPoint {
    GORotation = 0,

    _Count,
};

enum class TextureBindPoint {
    Sprite = 0,

    _Count,
};

enum class RectBindPoint {
    Sprite = 0,

    _Count,
};

using TextureTrack = Track<Texture*, false>;
using RectTrack = Track<Rect, false>;
using Vec2Track = Track<Vec2, true>;
using FloatTrack = Track<float, true>;

using TextureTrackPlayInfo = TrackPlayInfo<Texture*, false>;
using RectTrackPlayInfo = TrackPlayInfo<Rect, false>;
using Vec2TrackPlayInfo = TrackPlayInfo<Vec2, true>;
using FloatTrackPlayInfo = TrackPlayInfo<float, true>;

static constexpr int InfLoop = -1;

class Animation {
public:
    explicit Animation(const std::string& filename);

    std::array<Vec2Track, static_cast<size_t>(Vec2BindPoint::_Count)>
        vec2Tracks;
    std::array<FloatTrack, static_cast<size_t>(FloatBindPoint::_Count)>
        floatTracks;
    std::array<RectTrack, static_cast<size_t>(RectBindPoint::_Count)>
        rectTracks;
    std::array<TextureTrack, static_cast<size_t>(TextureBindPoint::_Count)>
        textureTracks;

    TimeType GetMaxTime() const { return maxTime_; }

    operator bool() const noexcept;

private:
    TimeType maxTime_ = 0;
    bool isValid_ = false;

    TimeType findMaxTime() const;

    bool parseVec2Track(const tinyxml2::XMLElement&, Vec2BindPoint&,
                        Vec2Track&) const;
    bool parseFloatTrack(const tinyxml2::XMLElement&, FloatBindPoint&,
                         FloatTrack&) const;
    bool parseRectTrack(const tinyxml2::XMLElement&, RectBindPoint&,
                        RectTrack&) const;
    bool parseTextureTrack(const tinyxml2::XMLElement&, TextureBindPoint&,
                           TextureTrack&) const;
    bool parseDataAttributes(const tinyxml2::XMLElement&, TimeType& time,
                             Interpolate& interp) const;
};

struct Animator {
    friend class Scene;

    Animation* animation = nullptr;

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    bool IsPlaying() const;

    void SetLoop(int loop);
    int GetLoop() const;
    void SetRate(float rate);
    float GetRate() const;

    float GetCurTime() const { return curTime_; }

    void Update(uint32_t elapse);

    operator bool() const { return animation; }

private:
    std::array<Vec2TrackPlayInfo, static_cast<size_t>(Vec2BindPoint::_Count)>
        vec2Tracks_;
    std::array<FloatTrackPlayInfo, static_cast<size_t>(FloatBindPoint::_Count)>
        floatTracks_;
    std::array<RectTrackPlayInfo, static_cast<size_t>(RectBindPoint::_Count)>
        rectTracks_;
    std::array<TextureTrackPlayInfo,
               static_cast<size_t>(TextureBindPoint::_Count)>
        textureTracks_;

    bool isPlaying_ = false;
    int loop_ = 0;
    float curTime_ = 0;
    float rate_ = 1;

    template <typename T, bool CanInterp>
    void updateTrack(Track<T, CanInterp>& track, TrackPlayInfo<T, CanInterp>& playInfo) {
        TL_RETURN_IF_FALSE(!track.keyframes.empty());
        TL_RETURN_IF_FALSE(curTime_ >= track.keyframes[0].time &&
                     curTime_ <= track.keyframes.back().time);

        if (rate_ > 0) {
            while (playInfo.curFrame + 1 < track.keyframes.size() &&
                   track.keyframes[playInfo.curFrame + 1].time <= curTime_) {
                playInfo.curFrame++;
            }
        } else if (rate_ < 0) {
            while (playInfo.curFrame > 0 &&
                   track.keyframes[playInfo.curFrame].time > curTime_) {
                playInfo.curFrame--;
            }
        }

        if (playInfo.curFrame + 1 == track.keyframes.size()) {
            playInfo.curData = track.keyframes.back().value;
            return;
        }

        auto& prevFrame = track.keyframes[playInfo.curFrame];
        auto& nextFrame = track.keyframes[playInfo.curFrame + 1];
        if constexpr (CanInterp) {
            if (prevFrame.interpolate == Interpolate::Linear) {
                float percent = (curTime_ - prevFrame.time) /
                                float(nextFrame.time - prevFrame.time);
                T value = prevFrame.value +
                          (nextFrame.value - prevFrame.value) * percent;
                playInfo.curData = value;
            } else {
                playInfo.curData = prevFrame.value;
            }
        } else {
            playInfo.curData = prevFrame.value;
        }
    }

    template <typename T, bool CanInterp>
    void updateTrackCurFrame(Track<T, CanInterp>& track, TrackPlayInfo<T, CanInterp>& playInfo) {
        TL_RETURN_IF_FALSE(!track.keyframes.empty());

        if (rate_ > 0) {
            int i = 0;
            for (; i + 1 < track.keyframes.size(); i++) {
                auto& prevFrame = track.keyframes[i];
                auto& nextFrame = track.keyframes[i + 1];
                if (prevFrame.time <= curTime_ && nextFrame.time > curTime_) {
                    playInfo.curFrame = i;
                    return;
                }
            }
            playInfo.curFrame = i;
        } else {
            int i = track.keyframes.size() - 1;
            for (; i - 1 > 0; i--) {
                auto& prevFrame = track.keyframes[i - 1];
                auto& nextFrame = track.keyframes[i];
                if (prevFrame.time < curTime_ && nextFrame.time >= curTime_) {
                    playInfo.curFrame = i - 1;
                    return;
                }
            }
            playInfo.curFrame = i > 0 ? i - 1 : 0;
        }
    }

    void updateAllTrack();
};

class AnimationManager {
public:
    Animation* Load(const std::string& filename, const std::string& name);
    Animation* Find(const std::string& name);
    const Animation* Find(const std::string& name) const;
    void Destroy(const std::string& name);
    void Clear();

private:
    std::unordered_map<std::string, Animation> animations_;
};

}  // namespace tl
