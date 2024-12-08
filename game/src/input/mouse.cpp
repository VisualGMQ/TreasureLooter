#include "input/mouse.hpp"
#include "macro.hpp"
#include "profile.hpp"

namespace tl::input {

void Mouse::Button::HandleEvent(const SDL_MouseButtonEvent& event) {
    TL_RETURN_IF_FALSE(event.button == static_cast<int>(type_));

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        isPressing_ = true;
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        isPressing_ = false;
    }
}

void Mouse::Button::Update() {
    isPressed_ = isPressing_;
}

const Mouse::Button& Mouse::GetButton(Mouse::Button::Type type) const {
    return buttons_[static_cast<int>(type) - 1];
}

Mouse::Mouse() {
    for (int i = 0; i < 5; i++) {
        buttons_[i].type_ = static_cast<Mouse::Button::Type>(i + 1);
    }
}

void Mouse::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        buttons_[event.button.button - 1].HandleEvent(event.button);
    }

    if (event.type == SDL_MOUSEMOTION) {
        curPos_.x = event.motion.x;
        curPos_.y = event.motion.y;
        offset_.x = event.motion.xrel;
        offset_.y = event.motion.yrel;
    }
}

const Vec2& Mouse::GetPosition() const {
    return curPos_;
}

const Vec2& Mouse::GetOffset() const {
    return offset_;
}

void Mouse::Update() {
    PROFILE_FUNC();
    for (auto& btn : buttons_) {
        btn.Update();
    }
}

}  // namespace tl