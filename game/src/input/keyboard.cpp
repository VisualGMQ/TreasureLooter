#include "input/keyboard.hpp"
#include "macro.hpp"

namespace tl::input {

Keyboard::Keyboard() {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        buttons_[i].key_ = static_cast<SDL_Scancode>(i);
    }
}

void Keyboard::Button::HandleEvent(const SDL_KeyboardEvent& event) {
    TL_RETURN_IF(event.keysym.scancode == key_);

    if (event.type == SDL_KEYDOWN) {
        isPressing_ = true;
    } else if (event.type == SDL_KEYUP) {
        isPressing_ = false;
    }
}

void Keyboard::Button::Update() {
    isPressed_ = isPressing_;
}

const Keyboard::Button& Keyboard::GetKey(SDL_Scancode key) const {
    return buttons_[key];
}

void Keyboard::HandleEvent(const SDL_Event& event) {
    TL_RETURN_IF(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP);

    buttons_[event.key.keysym.scancode].HandleEvent(event.key);
}

void Keyboard::Update() {
    for (auto& btn : buttons_) {
        btn.Update();
    }
}

}  // namespace tl