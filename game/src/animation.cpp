#include "animation.hpp"
#include "common.hpp"
#include "context.hpp"

namespace tl {

Animation::Animation(const std::string& filename) {
    size_t fileSize;
    void* fileContent = SDL_LoadFile(filename.c_str(), &fileSize);
    TL_RETURN_IF_FALSE_LOGE(fileContent, "%s load failed", filename.c_str());

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.Parse((const char*)fileContent, fileSize);
    TL_RETURN_IF_FALSE_LOGE(!err, "animation %s load failed", filename.c_str());

    auto animElem = doc.FirstChildElement("animation");
    TL_RETURN_IF_FALSE(animElem);

    auto tracksElem = animElem->FirstChildElement("tracks");
    TL_RETURN_IF_FALSE(tracksElem);

    auto trackElem = tracksElem->FirstChildElement("track");
    while (trackElem) {
        auto bindElem = trackElem->FirstChildElement("bind");
        TL_RETURN_IF_FALSE(bindElem);

        std::string_view bind = bindElem->GetText();
        if (bind == "go_position" || bind == "go_scale") {
            Vec2BindPoint bindPoint;
            Vec2Track track;
            TL_RETURN_IF_FALSE(parseVec2Track(*trackElem, bindPoint, track));
            vec2Tracks[static_cast<size_t>(bindPoint)] = std::move(track);
        } else if (bind == "go_rotation") {
            FloatBindPoint bindPoint;
            FloatTrack track;
            TL_RETURN_IF_FALSE(parseFloatTrack(*trackElem, bindPoint, track));
            floatTracks[static_cast<size_t>(bindPoint)] = std::move(track);
        } else if (bind == "sprite_region") {
            RectBindPoint bindPoint;
            RectTrack track;
            TL_RETURN_IF_FALSE(parseRectTrack(*trackElem, bindPoint, track));
            rectTracks[static_cast<size_t>(bindPoint)] = std::move(track);
        } else if (bind == "sprite_texture") {
            TextureBindPoint bindPoint;
            TextureTrack track;
            TL_RETURN_IF_FALSE(parseTextureTrack(*trackElem, bindPoint, track));
            textureTracks[static_cast<size_t>(bindPoint)] = std::move(track);
        }

        auto sibling = trackElem->NextSibling();
        TL_BREAK_IF_FALSE(sibling);
        trackElem = sibling->ToElement();
    }

    maxTime_ = findMaxTime();
}

bool Animation::parseVec2Track(const tinyxml2::XMLElement& elem,
                               Vec2BindPoint& bindPoint,
                               Vec2Track& track) const {
    auto bindElem = elem.FirstChildElement("bind");
    TL_RETURN_FALSE_IF_FALSE(bindElem);

    if (strcmp(bindElem->GetText(), "go_position") == 0) {
        bindPoint = Vec2BindPoint::GOPosition;
    } else if (strcmp(bindElem->GetText(), "go_scale") == 0) {
        bindPoint = Vec2BindPoint::GOScale;
    }

    auto dataElem = elem.FirstChildElement("data");
    TL_RETURN_FALSE_IF_FALSE(dataElem);

    auto element = dataElem->FirstChildElement("element");
    while (element) {
        Interpolate interpolate;
        TimeType time;
        parseDataAttributes(*element, time, interpolate);

        Vec2 value;
        ParseFloat(element->GetText(), (float*)&value, 2);

        track.keyframes.emplace_back(value, time, interpolate);

        auto sibling = element->NextSibling();
        TL_BREAK_IF_FALSE(sibling);
        element = sibling->ToElement();
    }

    return true;
}

bool Animation::parseFloatTrack(const tinyxml2::XMLElement& elem,
                                FloatBindPoint& bindPoint,
                                FloatTrack& track) const {
    auto bindElem = elem.FirstChildElement("bind");
    TL_RETURN_FALSE_IF_FALSE(bindElem);

    if (strcmp(bindElem->GetText(), "go_rotation") == 0) {
        bindPoint = FloatBindPoint::GORotation;
    }

    auto dataElem = elem.FirstChildElement("data");
    TL_RETURN_FALSE_IF_FALSE(dataElem);

    auto element = dataElem->FirstChildElement("element");
    while (element) {
        Interpolate interpolate;
        TimeType time;
        parseDataAttributes(*element, time, interpolate);

        float value;
        ParseFloat(element->GetText(), &value, 1);

        track.keyframes.emplace_back(value, time, interpolate);

        auto sibling = element->NextSibling();
        TL_BREAK_IF_FALSE(sibling);
        element = sibling->ToElement();
    }

    return true;
}

bool Animation::parseRectTrack(const tinyxml2::XMLElement& elem,
                               RectBindPoint& bindPoint,
                               RectTrack& track) const {
    auto bindElem = elem.FirstChildElement("bind");
    TL_RETURN_FALSE_IF_FALSE(bindElem);

    if (strcmp(bindElem->GetText(), "sprite_region") == 0) {
        bindPoint = RectBindPoint::Sprite;
    }

    auto dataElem = elem.FirstChildElement("data");
    TL_RETURN_FALSE_IF_FALSE(dataElem);

    auto element = dataElem->FirstChildElement("element");
    while (element) {
        Interpolate interpolate;
        TimeType time;
        parseDataAttributes(*element, time, interpolate);

        Rect value;
        ParseFloat(element->GetText(), (float*)&value, 4);

        track.keyframes.emplace_back(value, time, interpolate);

        auto sibling = element->NextSibling();
        TL_BREAK_IF_FALSE(sibling);
        element = sibling->ToElement();
    }

    return true;
}

bool Animation::parseTextureTrack(const tinyxml2::XMLElement& elem,
                                  TextureBindPoint& bindPoint,
                                  TextureTrack& track) const {
    auto bindElem = elem.FirstChildElement("bind");
    TL_RETURN_FALSE_IF_FALSE(bindElem);

    if (strcmp(bindElem->GetText(), "sprite_texture") == 0) {
        bindPoint = TextureBindPoint::Sprite;
    }

    auto dataElem = elem.FirstChildElement("data");
    TL_RETURN_FALSE_IF_FALSE(dataElem);

    auto element = dataElem->FirstChildElement("element");
    while (element) {
        Interpolate interpolate;
        TimeType time;
        parseDataAttributes(*element, time, interpolate);

        std::string_view textureName = element->GetText();
        Texture* texture = Context::GetInst().textureMgr->Find(
            std::string(element->GetText()));
        TL_RETURN_FALSE_IF_FALSE(texture);

        track.keyframes.emplace_back(texture, time, interpolate);

        auto sibling = element->NextSibling();
        TL_BREAK_IF_FALSE(sibling);
        element = sibling->ToElement();
    }

    return true;
}

bool Animation::parseDataAttributes(const tinyxml2::XMLElement& element,
                                    TimeType& time,
                                    Interpolate& interpolate) const {
    auto timeAttr = element.FindAttribute("time");
    TL_RETURN_FALSE_IF_FALSE(timeAttr);
    time = timeAttr->Int64Value();

    interpolate = Interpolate::Linear;
    auto interpAttr = element.FindAttribute("interpolate");

    TL_RETURN_FALSE_IF_FALSE(interpAttr);
    if (strcmp(interpAttr->Value(), "linear") == 0) {
        interpolate = Interpolate::Linear;
    } else if (strcmp(interpAttr->Value(), "none") == 0) {
        interpolate = Interpolate::None;
    } else {
        interpolate = Interpolate::Linear;
        LOGW("unknown interpolate type %s", interpAttr->Value());
    }

    return true;
}

TimeType Animation::findMaxTime() const {
    uint32_t maxTime = 0;
    for (auto& track : vec2Tracks) {
        TL_CONTINUE_IF_FALSE(!track.keyframes.empty());

        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }
    for (auto& track : floatTracks) {
        TL_CONTINUE_IF_FALSE(!track.keyframes.empty());

        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }
    for (auto& track : textureTracks) {
        TL_CONTINUE_IF_FALSE(!track.keyframes.empty());

        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }
    for (auto& track : rectTracks) {
        TL_CONTINUE_IF_FALSE(!track.keyframes.empty());

        uint32_t trackMaxTime = track.keyframes.back().time;
        maxTime = std::max(trackMaxTime, maxTime);
    }

    return maxTime;
}

void Animator::Play() {
    isPlaying_ = true;
}

void Animator::Pause() {
    isPlaying_ = false;
}

void Animator::Rewind() {
    curTime_ = 0;

    for (auto& track : vec2Tracks_) {
        track.curFrame = 0;
    }

    for (auto& track : floatTracks_) {
        track.curFrame = 0;
    }

    for (auto& track : textureTracks_) {
        track.curFrame = 0;
    }

    for (auto& track : rectTracks_) {
        track.curFrame = 0;
    }
}

void Animator::Stop() {
    Pause();
    Rewind();
}

bool Animator::IsPlaying() const {
    return isPlaying_;
}

void Animator::SetLoop(int loop) {
    if (loop_ < 0) {
        loop_ = InfLoop;
    }
    loop_ = loop;
}

int Animator::GetLoop() const {
    return loop_;
}

void Animator::SetRate(float value) {
    rate_ = value;
}

float Animator::GetRate() const {
    return rate_;
}

void Animator::Update(uint32_t elapse) {
    TL_RETURN_IF_FALSE(rate_ != 0 && isPlaying_ && animation);

    updateAllTrack();

    curTime_ += elapse * rate_;

    TimeType maxTime = animation->GetMaxTime();

    if (curTime_ >= animation->GetMaxTime() || curTime_ <= 0) {
        if (loop_ == 0) {
            isPlaying_ = false;
            curTime_ = Clamp<float>(0, maxTime, curTime_);

            updateAllTrack();
        } else {
            curTime_ = rate_ > 0 ? curTime_ - maxTime : curTime_;
            curTime_ = rate_ < 0 ? curTime_ + maxTime : curTime_;

            loop_ = loop_ == InfLoop ? InfLoop : loop_ - 1;

            for (int i = 0; i < static_cast<int>(Vec2BindPoint::_Count); i++) {
                updateTrackCurFrame(animation->vec2Tracks[i], vec2Tracks_[i]);
            }
            for (int i = 0; i < static_cast<int>(FloatBindPoint::_Count); i++) {
                updateTrackCurFrame(animation->floatTracks[i], floatTracks_[i]);
            }
            for (int i = 0; i < static_cast<int>(TextureBindPoint::_Count);
                 i++) {
                updateTrackCurFrame(animation->textureTracks[i],
                                    textureTracks_[i]);
            }
            for (int i = 0; i < static_cast<int>(RectBindPoint::_Count); i++) {
                updateTrackCurFrame(animation->rectTracks[i], rectTracks_[i]);
            }
        }
    }
}

void Animator::updateAllTrack() {
    for (int i = 0; i < static_cast<int>(Vec2BindPoint::_Count); i++) {
        updateTrack(animation->vec2Tracks[i], vec2Tracks_[i]);
    }
    for (int i = 0; i < static_cast<int>(FloatBindPoint::_Count); i++) {
        updateTrack(animation->floatTracks[i], floatTracks_[i]);
    }
    for (int i = 0; i < static_cast<int>(TextureBindPoint::_Count); i++) {
        updateTrack(animation->textureTracks[i], textureTracks_[i]);
    }
    for (int i = 0; i < static_cast<int>(RectBindPoint::_Count); i++) {
        updateTrack(animation->rectTracks[i], rectTracks_[i]);
    }
}

Animation* AnimationManager::Load(const std::string& filename,
                                  const std::string& name) {
    auto result = animations_.emplace(name, filename);
    TL_RETURN_NULL_IF_FALSE_LOGE(result.second, "load animation %s failed",
                           name.c_str());

    return &result.first->second;
}

Animation* AnimationManager::Find(const std::string& name) {
    return const_cast<Animation*>(std::as_const(*this).Find(name));
}

const Animation* AnimationManager::Find(const std::string& name) const {
    if (auto it = animations_.find(name); it != animations_.end()) {
        return &it->second;
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
