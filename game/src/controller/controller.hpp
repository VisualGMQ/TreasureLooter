#pragma once
#include "math.hpp"
#include "pch.hpp"

namespace tl::controller {

class Controller {
public:
    enum class Type {
        Keyboard,
        GameController,
        Touch,
    };

    struct Button {
        enum class Type {
            Pressed,
            Pressing,
            Released,
            Releasing,
        };

        Button() : type_{Type::Releasing} {}
        explicit Button(Type type) : type_{type} {}

        bool IsPressed() const { return type_ == Type::Pressed; }

        bool IsPressing() const { return type_ == Type::Pressing; }

        bool IsReleased() const { return type_ == Type::Released; }

        bool IsReleasing() const { return type_ == Type::Releasing; }

    private:
        Type type_;
    };

    explicit Controller(Type type): type_{type} {}
    virtual ~Controller() = default;

    virtual Vec2 GetAxis() const = 0;
    virtual Button GetAttackButton() const = 0;
    virtual Button GetDefendButton() const = 0;
    virtual Button GetInteractButton() const = 0;
    virtual void Update() {}

    Type GetType() const { return type_; }

private:
    Type type_;
};

class ControllerManager {
public:
    ControllerManager();
    void ChangeController(std::unique_ptr<Controller>&&);
    const Controller* GetController() const { return controller_.get(); }
    void HandleEvent(const SDL_Event&);
    void Update();

private:
    std::unique_ptr<Controller> controller_;
};

}  // namespace tl