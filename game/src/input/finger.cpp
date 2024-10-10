#include "input/finger.hpp"
#include "macro.hpp"

namespace tl::input {

void FingerManager::TouchPoint::HandleEvent(const SDL_TouchFingerEvent& event) {
    curPos_.x = event.x;
    curPos_.y = event.y;
    offset_.x = event.dx;
    offset_.y = event.dy;
    pressure_ = event.pressure;

    if (event.type == SDL_FINGERDOWN) {
        isPressing_ = true;
    }
    if (event.type == SDL_FINGERUP) {
        isPressing_ = false;
    }
}

void FingerManager::TouchPoint::Update() {
    isPressed_ = isPressing_;
}

void FingerManager::HandleEvent(const SDL_Event& event) {
    TL_RETURN_IF_FALSE(event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP ||
                 event.type == SDL_FINGERMOTION);
    auto id = event.tfinger.fingerId;
    TL_RETURN_IF_FALSE(id >= 0 && id < MaxFingerCount);

    fingers_[id].HandleEvent(event.tfinger);
}

void FingerManager::Update() {
    for (auto& finger : fingers_) {
        finger.Update();
    }
}

}  // namespace tl