#include "input/game_controller.hpp"
#include "macro.hpp"
#include "profile.hpp"

namespace tl::input {

void GameController::Button::HandleEvent(
    const SDL_ControllerButtonEvent& event) {
    Type type = static_cast<Type>(event.button);
    TL_RETURN_IF_FALSE(type != Type::Invalid);

    if (event.state == SDL_PRESSED) {
        isPressing_ = true;
    } else if (event.state == SDL_RELEASED) {
        isPressing_ = false;
    }
}

void GameController::Button::Update() {
    isPressed_ = isPressing_;
}

void GameController::Axis::HandleEvent(const SDL_ControllerAxisEvent& event) {
    if (event.value > 0) {
        value_ = event.value / 32767.0f;
    } else {
        value_ = event.value / 32768.0f;
    }
}

GameController::GameController(int index) {
    instanceID_ = SDL_JoystickGetDeviceInstanceID(index);
    TL_RETURN_IF_FALSE_LOGE(instanceID_ != -1, "game controller %d is not valid",
                      index);

    controller_ = SDL_GameControllerOpen(index);
    TL_RETURN_IF_FALSE_LOGE(controller_, "controller can't be open");

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        buttons_[i].type_ = static_cast<Button::Type>(i);
    }
    for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++) {
        axis_[i].type_ = static_cast<Axis::Type>(i);
    }
}

void GameController::HandleEvent(const SDL_Event& event) {
    if ((event.type == SDL_CONTROLLERBUTTONDOWN ||
         event.type == SDL_CONTROLLERBUTTONUP) &&
        event.cbutton.which == instanceID_) {
        buttons_[event.cbutton.button].HandleEvent(event.cbutton);
    }

    if (event.type == SDL_CONTROLLERAXISMOTION &&
        event.cbutton.which == instanceID_) {
        axis_[event.caxis.axis].HandleEvent(event.caxis);
    }
}

GameController::~GameController() {
    SDL_GameControllerClose(controller_);
}

const GameController::Axis& GameController::GetAxis(Axis::Type type) const {
    return axis_[static_cast<size_t>(type)];
}

const GameController::Button& GameController::GetButton(
    Button::Type type) const {
    return buttons_[static_cast<size_t>(type)];
}

void GameController::Update() {
    for (auto& btn : buttons_) {
        btn.Update();
    }
}

SDL_JoystickID GameController::GetID() const {
    return instanceID_;
}

void GameControllerManager::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_CONTROLLERDEVICEADDED) {
        int index = event.cdevice.which;
        GameController controller{index};
        SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(index);
        controllers_.emplace(id, id);
    } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
        controllers_.erase(event.cdevice.which);
    } else if (event.type == SDL_CONTROLLERDEVICEREMAPPED) {
        // TODO: not finish
    } else {
        for (auto& [_, controller] : controllers_) {
            controller.HandleEvent(event);
        }
    }
}

SDL_JoystickID GameControllerManager::GetJoystickID(int index) {
    return SDL_JoystickGetDeviceInstanceID(index);
}

void GameControllerManager::Update() {
    PROFILE_FUNC(); 
    
    for (auto& [_, controller] : controllers_) {
        controller.Update();
    }
}

}  // namespace tl::input