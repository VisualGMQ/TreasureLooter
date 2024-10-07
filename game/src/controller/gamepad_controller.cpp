#include "gamepad_controller.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl::controller {

GamePadController::GamePadController(const input::GameController& controller)
    : Controller{Controller::Type::GameController} {
    controller_ = &controller;
}

SDL_JoystickID GamePadController::GetID() const {
    return id_;
}

Vec2 GamePadController::GetAxis() const {
    if (!controller_) {
        return {};
    }

    Vec2 axis;
    axis.x = controller_->GetAxis(input::GameController::Axis::Type::LeftX)
                 .GetValue();
    axis.y = controller_->GetAxis(input::GameController::Axis::Type::LeftY)
                 .GetValue();
    return axis;
}

Controller::Button GamePadController::GetAttackButton() const {
    if (!controller_) {
        return {};
    }

    return cvtButton(input::GameController::Button::Type::X);
}

Controller::Button GamePadController::GetDefendButton() const {
    if (!controller_) {
        return {};
    }
    return cvtButton(input::GameController::Button::Type::A);
}

Controller::Button GamePadController::GetInteractButton() const {
    if (!controller_) {
        return {};
    }
    return cvtButton(input::GameController::Button::Type::Y);
}

Controller::Button GamePadController::cvtButton(
    input::GameController::Button::Type btnType) const {
    auto& btn = controller_->GetButton(btnType);
    Button::Type type;
    if (btn.IsPressed()) {
        type = Button::Type::Pressed;
    } else if (btn.IsPressing()) {
        type = Button::Type::Pressing;
    } else if (btn.IsReleased()) {
        type = Button::Type::Released;
    } else if (btn.IsReleasing()) {
        type = Button::Type::Releasing;
    }

    return Controller::Button{type};
}

}  // namespace tl::controller