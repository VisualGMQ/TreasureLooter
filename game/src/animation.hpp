#pragma once
#include "math.hpp"
#include "pch.hpp"
#include "texture.hpp"

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
    uint32_t time;
    Interpolate interpolate = Interpolate::Linear;
};

template <typename T, bool CanInterp>
struct Track {
    std::vector<Keyframe<T, CanInterp>> keyframes;
    T curData;
    uint32_t curFrame = 0;
};

enum class Vec2BindPoint {
    GOPosition,
    GOScale,
};

enum class FloatBindPoint {
    GORotation,
};

enum class TextureBindPoint {
    Sprite,
};

enum class RectBindPoint {
    Sprite,
};

using TextureTrack = Track<Texture*, false>;
using RectTrack = Track<Rect, false>;
using Vec2Track = Track<Vec2, true>;
using FloatTrack = Track<float, true>;

class Animation {
public:
    static constexpr int InfLoop = -1;

    void Play();
    void Pause();
    void Stop();
    void Rewind();
    bool IsPlaying() const;

    void SetLoop(int loop);
    int GetLoop() const;
    void SetRate(float rate);
    float GetRate() const;

    void Update(uint32_t elapse);

    void SetVec2Track(Vec2BindPoint, const Vec2Track&);
    void SetFloatTrack(FloatBindPoint, const FloatTrack&);
    void SetTextureTrack(TextureBindPoint, const TextureTrack&);
    void SetRectTrack(RectBindPoint bind, const RectTrack& track);

    auto& GetVec2Tracks() const { return vec2Tracks_; }

    auto& GetFloatTracks() const { return floatTracks_; }

    auto& GetTextureTracks() const { return textureTracks_; }

    auto& GetRectTracks() const { return rectTracks_; }

private:
    std::unordered_map<Vec2BindPoint, Vec2Track> vec2Tracks_;
    std::unordered_map<FloatBindPoint, FloatTrack> floatTracks_;
    std::unordered_map<RectBindPoint, RectTrack> rectTracks_;
    std::unordered_map<TextureBindPoint, TextureTrack> textureTracks_;
    float curTime_ = 0;
    int loop_ = 0;
    bool isPlaying_ = false;
    uint32_t maxTime_ = 0;
    float rate = 1;

    uint32_t findMaxTime() const;

    template <typename T, bool CanInterp>
    void updateTrack(Track<T, CanInterp>& track) {
        if (track.keyframes.empty()) {
            return;
        }

        if (rate > 0) {
            while (track.curFrame + 1 < track.keyframes.size() &&
                   track.keyframes[track.curFrame + 1].time <= curTime_) {
                track.curFrame++;
            }
        } else if (rate < 0) {
            while (track.curFrame > 0 &&
                   track.keyframes[track.curFrame].time > curTime_) {
                track.curFrame--;
            }
        }

        if (track.curFrame + 1 == track.keyframes.size()) {
            track.curData = track.keyframes.back().value;
            return;
        }

        auto& prevFrame = track.keyframes[track.curFrame];
        auto& nextFrame = track.keyframes[track.curFrame + 1];
        if constexpr (CanInterp) {
            if (prevFrame.interpolate == Interpolate::Linear) {
                float percent = (curTime_ - prevFrame.time) /
                                float(nextFrame.time - prevFrame.time);
                T value = prevFrame.value +
                          (nextFrame.value - prevFrame.value) * percent;
                track.curData = value;
            } else {
                track.curData = prevFrame.value;
            }
        } else {
            track.curData = prevFrame.value;
        }
    }

    template <typename T, bool CanInterp>
    void updateTrackCurFrame(Track<T, CanInterp>& track) {
        if (track.keyframes.empty()) {
            return;
        }

        if (rate > 0) {
            int i = 0;
            for (; i + 1 < track.keyframes.size(); i++) {
                auto& prevFrame = track.keyframes[i];
                auto& nextFrame = track.keyframes[i + 1];
                if (prevFrame.time <= curTime_ && nextFrame.time > curTime_) {
                    track.curFrame = i;
                    return;
                }
            }
            track.curFrame = i;
        } else {
            int i = track.keyframes.size() - 1;
            for (; i - 1 > 0; i--) {
                auto& prevFrame = track.keyframes[i - 1];
                auto& nextFrame = track.keyframes[i];
                if (prevFrame.time < curTime_ && nextFrame.time >= curTime_) {
                    track.curFrame = i - 1;
                    return;
                }
            }
            track.curFrame = i > 0 ? i - 1 : 0;
        }
    }

    void updateAllTrack();
};

struct Animator {
    Animation* animation = nullptr;

    operator bool() const { return animation; }
};

class AnimationManager {
public:
    Animation* Create(const std::string& name);
    Animation* Get(const std::string& name) const;
    void Destroy(const std::string& name);
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Animation>> animations_;
};

}  // namespace tl
