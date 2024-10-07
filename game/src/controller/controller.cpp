#include "controller/controller.hpp"
#include "context.hpp"
#include "controller/gamepad_controller.hpp"
#include "controller/keyboard_controller.hpp"
#include "controller/touch_controller.hpp"

namespace tl::controller {

ControllerManager::ControllerManager() {
    auto& gameCtrlMgr = Context::GetInst().gameCtrlMgr;
    if (!gameCtrlMgr->GetControllers().empty()) {
        controller_ = std::make_unique<GamePadController>(
            gameCtrlMgr->GetControllers().begin()->second);
    } else {
#ifdef TL_ANDROID
        controller_ = std::make_unique<TouchController>();
#else
        controller_ = std::make_unique<KeyboardController>();
#endif
    }
}

void ControllerManager::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_CONTROLLERDEVICEADDED) {
        SDL_JoystickID id =
            input::GameControllerManager::GetJoystickID(event.cdevice.which);
        if (id != -1) {
            auto& controllers =
                Context::GetInst().gameCtrlMgr->GetControllers();
            auto it = controllers.find(id);
            if (it != controllers.end()) {
                ChangeController(
                    std::make_unique<GamePadController>(it->second));
            }
        }
    } else if (event.type == SDL_CONTROLLERDEVICEREMOVED &&
               controller_->GetType() == Controller::Type::GameController &&
               ((GamePadController*)&controller_)->GetID() ==
                   event.cdevice.which) {
#ifdef TL_ANDROID
        ChangeController(std::make_unique<TouchController>());
#else
        ChangeController(std::make_unique<KeyboardController>());
#endif
    }
}

void ControllerManager::ChangeController(
    std::unique_ptr<Controller>&& controller) {
    controller_ = std::move(controller);
}

void ControllerManager::Update() {
    if (controller_) {
        controller_->Update();
    }
}

}  // namespace tl::controller