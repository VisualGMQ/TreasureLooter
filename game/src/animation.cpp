#include "animation.hpp"

namespace tl {

void Animation::Play() {
    isPlaying_ = true;
}

void Animation::Pause() {
    isPlaying_ = false;
}

void Animation::Rewind() {
    curTime_ = 0;
}

void Animation::Stop() {
    Pause();
    Stop();
}

bool Animation::IsPlaying() const {
    return isPlaying_;
}

void Animation::SetLoop(int loop) {
    if (loop_ < 0) {
        loop_ = InfLoop;
    }
    loop_ = loop;
}

int Animation::GetLoop() const {
    return loop_;
}

void Animation::SetRate(float value) {
    rate = value;
}

float Animation::GetRate() const {
    return rate;
}

void Animation::SetVec2Track(Vec2BindPoint bind, const Vec2Track& track) {
    if (track.keyframes.empty()) {
        return;
    }
    Vec2& value = vec2Tracks_.emplace(bind, track).first->second.curData;
    value = track.keyframes[0].value;
    maxTime_ = findMaxTime();
}

void Animation::SetFloatTrack(FloatBindPoint bind, const FloatTrack& track) {
    if (track.keyframes.empty()) {
        return;
    }
    auto& t = floatTracks_.emplace(bind, track).first->second;
    t.curData = t.keyframes[0].value;
    maxTime_ = findMaxTime();
}

void Animation::SetTextureTrack(TextureBindPoint bind,
                                const TextureTrack& track) {
    if (track.keyframes.empty()) {
        return;
    }
    auto& t = textureTracks_.emplace(bind, track).first->second;
    t.curData = t.keyframes[0].value;
    maxTime_ = findMaxTime();
}

void Animation::SetRectTrack(RectBindPoint bind, const RectTrack& track) {
    if (track.keyframes.empty()) {
        return;
    }
    auto& t = rectTracks_.emplace(bind, track).first->second;
    t.curData = t.keyframes[0].value;
    maxTime_ = findMaxTime();
}

uint32_t Animation::findMaxTime() const {
    uint32_t maxTime = 0;
    for (auto& [_, track] : vec2Tracks_) {
        if (track.keyframes.empty()) {
            continue;
        }
        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }
    for (auto& [_, track] : floatTracks_) {
        if (track.keyframes.empty()) {
            continue;
        }
        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }
    for (auto& [_, track] : textureTracks_) {
        if (track.keyframes.empty()) {
            continue;
        }
        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }

    return maxTime;
}

void Animation::Update(uint32_t elapse) {
    if (rate == 0 || !isPlaying_) {
        return;
    }

    updateAllTrack();

    curTime_ += elapse * rate;

    if (curTime_ >= maxTime_ || curTime_ <= 0) {
        if (loop_ == 0) {
            isPlaying_ = false;
            curTime_ = Clamp<float>(0, maxTime_, curTime_);

            updateAllTrack();
        }
        if (loop_ > 0) {
            curTime_ = rate > 0 ? curTime_ - maxTime_ : curTime_;
            curTime_ = rate < 0 ? curTime_ + maxTime_ : curTime_;

            loop_ = loop_ == InfLoop ? InfLoop : loop_ - 1;

            for (auto& [_, track] : vec2Tracks_) {
                updateTrackCurFrame(track);
            }
            for (auto& [_, track] : floatTracks_) {
                updateTrackCurFrame(track);
            }
            for (auto& [_, track] : textureTracks_) {
                updateTrackCurFrame(track);
            }
            for (auto& [_, track] : rectTracks_) {
                updateTrackCurFrame(track);
            }
        }
    }
}

void Animation::updateAllTrack() {
    for (auto& [_, track] : vec2Tracks_) {
        updateTrack(track);
    }
    for (auto& [_, track] : floatTracks_) {
        updateTrack(track);
    }
    for (auto& [_, track] : textureTracks_) {
        updateTrack(track);
    }
    for (auto& [_, track] : rectTracks_) {
        updateTrack(track);
    }
}

Animation* AnimationManager::Create(const std::string& name) {
    std::unique_ptr<Animation> anim = std::make_unique<Animation>();
    if (!anim) {
        return nullptr;
    }

    return animations_.emplace(name, std::move(anim)).first->second.get();
}

Animation* AnimationManager::Get(const std::string& name) const {
    if (auto it = animations_.find(name); it != animations_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void AnimationManager::Destroy(const std::string& name) {
    animations_.erase(name);
}

void AnimationManager::Clear() {
    animations_.clear();
}

}  // namespace tl
