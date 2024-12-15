#include "controller/keyboard_controller.hpp"
#include "context.hpp"
#include "math.hpp"

namespace tl::controller {

KeyboardController::KeyboardController()
    : Controller{Controller::Type::Keyboard} {}

Vec2 KeyboardController::GetAxis() const {
    Vec2 axis;

    auto& keyboard = Context::GetInst().keyboard;
    {
        auto& btn = keyboard->GetKey(SDL_SCANCODE_A);
        if (btn.IsPressed() || btn.IsPressing()) {
            axis.x -= 1;
        }
    }

    {
        auto& btn = keyboard->GetKey(SDL_SCANCODE_D);
        if (btn.IsPressed() || btn.IsPressing()) {
            axis.x += 1;
        }
    }

    {
        auto& btn = keyboard->GetKey(SDL_SCANCODE_W);
        if (btn.IsPressed() || btn.IsPressing()) {
            axis.y -= 1;
        }
    }

    {
        auto& btn = keyboard->GetKey(SDL_SCANCODE_S);
        if (btn.IsPressed() || btn.IsPressing()) {
            axis.y += 1;
        }
    }

    if (axis.x != 0 && axis.y != 0) {
        axis.ToNormalize();
    }
    return axis;
}

Controller::Button KeyboardController::GetAttackButton() const {
    return cvtKey2Button(SDL_SCANCODE_J);
}

Controller::Button KeyboardController::GetDefendButton() const {
    return cvtKey2Button(SDL_SCANCODE_L);
}

Controller::Button KeyboardController::GetInteractButton() const {
    return cvtKey2Button(SDL_SCANCODE_K);
}

Controller::Button KeyboardController::cvtKey2Button(
    SDL_Scancode scancode) const {
    auto& keyboard = Context::GetInst().keyboard;
    auto& key = keyboard->GetKey(scancode);
    Controller::Button::Type type;
    if (key.IsPressed()) {
        type = Controller::Button::Type::Pressed;
    } else if (key.IsPressing()) {
        type = Controller::Button::Type::Pressing;
    } else if (key.IsReleased()) {
        type = Controller::Button::Type::Released;
    } else if (key.IsReleasing()) {
        type = Controller::Button::Type::Releasing;
    }

    return Controller::Button{type};
}

}  // namespace tl