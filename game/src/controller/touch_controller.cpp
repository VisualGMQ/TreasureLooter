#include "touch_controller.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl::controller {

TouchController::TouchController() : Controller{Controller::Type::Touch} {
    axisCircle_.radius = 50;
    axisCircle_.center = Vec2{100, 650};

    axisTouchCircle_.radius = 25;
    axisTouchCircle_.center = Vec2{100, 650};

    button1Circle_.radius = 30;
    button1Circle_.center = Vec2{700, 650};

    button2Circle_.radius = 30;
    button2Circle_.center = Vec2{800, 650};

    button3Circle_.radius = 30;
    button3Circle_.center = Vec2{900, 650};
}

Vec2 TouchController::GetAxis() const {
    if (fingerInAxisCircle_) {
#ifdef TL_ANDROID
        auto& fingerMgr = Context::GetInst().fingerMgr;
        auto& finger = fingerMgr->GetFinger(fingerInAxisCircle_.value());
        Vec2 curPos =
            finger.GetPosition() * Context::GetInst().window->GetSize();
#else
        Vec2 curPos = Context::GetInst().mouse->GetPosition();
#endif
        Vec2 dir = curPos - axisCircle_.center;
        float len = dir.Length();
        if (!FLT_EQ(len, 0)) {
            return dir / len * std::min(len, axisCircle_.radius) / axisCircle_.radius;
        }
    }
    return {};
}

Controller::Button TouchController::GetAttackButton() const {
    if (fingerInBtn1Circle_) {
        return cvtButton(fingerInBtn1Circle_.value());
    }
    return {};
}

Controller::Button TouchController::GetDefendButton() const {
    if (fingerInBtn2Circle_) {
        return cvtButton(fingerInBtn2Circle_.value());
    }
    return {};
}

Controller::Button TouchController::GetInteractButton() const {
    if (fingerInBtn3Circle_) {
        return cvtButton(fingerInBtn3Circle_.value());
    }
    return {};
}

Controller::Button TouchController::cvtButton(SDL_FingerID id) const {
#ifdef TL_ANDROID
    auto& button = Context::GetInst().fingerMgr->GetFinger(id);
#else
    auto& button =
        Context::GetInst().mouse->GetButton(input::Mouse::Button::Type::Left);
#endif

    Controller::Button::Type type;
    if (button.IsPressed()) {
        type = Controller::Button::Type::Pressed;
    } else if (button.IsPressing()) {
        type = Controller::Button::Type::Pressing;
    } else if (button.IsReleased()) {
        type = Controller::Button::Type::Released;
    } else if (button.IsReleasing()) {
        type = Controller::Button::Type::Releasing;
    }

    return Controller::Button{type};
}

void TouchController::Update() {
    updateTouchCircle();
    updateFingerState(fingerInAxisCircle_, axisCircle_);
    updateFingerState(fingerInBtn1Circle_, button1Circle_);
    updateFingerState(fingerInBtn2Circle_, button2Circle_);
    updateFingerState(fingerInBtn3Circle_, button3Circle_);
    renderVirtualButtons();
}

void TouchController::updateTouchCircle() {
    if (fingerInAxisCircle_) {
        auto& fingerMgr = Context::GetInst().fingerMgr;
        auto& finger = fingerMgr->GetFinger(fingerInAxisCircle_.value());

#ifdef TL_ANDROID
        Vec2 curPos =
            finger.GetPosition() * Context::GetInst().window->GetSize();
#else
        Vec2 curPos = Context::GetInst().mouse->GetPosition();
#endif
        Vec2 dir = curPos - axisCircle_.center;
        float len = dir.Length();
        if (!FLT_EQ(len, 0)) {
            axisTouchCircle_.center =
                dir / len * std::min(len, axisCircle_.radius) +
                axisCircle_.center;
            return;
        }
    }
    axisTouchCircle_.center = axisCircle_.center;
}

void TouchController::updateFingerState(std::optional<SDL_FingerID>& id,
                                        const Circle& circle) {
#ifdef TL_ANDROID
    auto& fingerMgr = Context::GetInst().fingerMgr;
    if (!id) {
        Vec2 windowSize = Context::GetInst().window->GetSize();
        for (int i = 0; i < fingerMgr->FingerMaxCount(); i++) {
            auto& button = fingerMgr->GetFinger(i);
            if (button.IsPressed() &&
                IsPointInCircle(button.GetPosition() * windowSize, circle)) {
                id = i;
                break;
            }
        }
    } else {
        auto& finger = fingerMgr->GetFinger(id.value());
        if (finger.IsReleasing() || finger.IsReleased()) {
            id = std::nullopt;
        }
    }
#else
    auto& mouse = Context::GetInst().mouse;
    auto& button = mouse->GetButton(input::Mouse::Button::Type::Left);
    if (button.IsPressed() && IsPointInCircle(mouse->GetPosition(), circle)) {
        id = 0;
    } else if (button.IsReleasing() || button.IsReleased()) {
        id = std::nullopt;
    }
#endif
}

void TouchController::renderVirtualButtons() {
    auto& renderer = Context::GetInst().renderer;
    renderer->DrawCircle(axisCircle_, {1, 0, 0});
    renderer->DrawCircle(axisTouchCircle_, {1, 0, 0});
    renderer->DrawCircle(button1Circle_, {0, 1, 0});
    renderer->DrawCircle(button2Circle_, {0, 1, 0});
    renderer->DrawCircle(button3Circle_, {0, 1, 0});
}

}  // namespace tl::controller