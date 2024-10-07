#pragma once
#include "controller/controller.hpp"

namespace tl::controller {

class TouchController: public Controller {
public:
    TouchController();
    Vec2 GetAxis() const override;
    Button GetAttackButton() const override;
    Button GetDefendButton() const override;
    Button GetInteractButton() const override;
    void Update() override;

private:
    Circle axisCircle_;
    Circle axisTouchCircle_;
    Circle button1Circle_;
    Circle button2Circle_;
    Circle button3Circle_;

    std::optional<SDL_FingerID> fingerInAxisCircle_;
    std::optional<SDL_FingerID> fingerInBtn1Circle_;
    std::optional<SDL_FingerID> fingerInBtn2Circle_;
    std::optional<SDL_FingerID> fingerInBtn3Circle_;

    void updateTouchCircle();
    Button cvtButton(SDL_FingerID) const;
    void updateFingerState(std::optional<SDL_FingerID>&, const Circle&);
    void renderVirtualButtons();
};

}  // namespace tl