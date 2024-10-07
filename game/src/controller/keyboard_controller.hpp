#pragma once

#include "controller/controller.hpp"

namespace tl::controller {

class KeyboardController : public Controller {
public:
    KeyboardController();
    Vec2 GetAxis() const override;
    Button GetAttackButton() const override;
    Button GetDefendButton() const override;
    Button GetInteractButton() const override;

private:
    Button cvtKey2Button(SDL_Scancode) const;
};

}  // namespace tl