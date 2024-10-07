#pragma once
#include "controller/controller.hpp"
#include "input/game_controller.hpp"

namespace tl::controller {

class GamePadController : public Controller {
public:
    explicit GamePadController(const input::GameController&);
    Vec2 GetAxis() const override;
    Button GetAttackButton() const override;
    Button GetDefendButton() const override;
    Button GetInteractButton() const override;
    SDL_JoystickID GetID() const;

private:
    Button cvtButton(input::GameController::Button::Type) const;
    const input::GameController* controller_ = nullptr;
    SDL_JoystickID id_;
};

}  // namespace tl