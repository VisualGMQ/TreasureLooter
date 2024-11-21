#pragma once
#include "game_controller.hpp"
#include "level.hpp"

namespace tl {

class CharacterController : public GameController {
public:
    void SetCharacter(GameObjectID);
    GameObject* GetCharacter();
    bool HasCharacter() const;
    void Update() override;

private:
    GameObjectID go_;
};

class TestLevel : public Level {
public:
    void Init() override {}
    void Enter() override;
    void Quit() override {}
    void Update() override {}
};

}  // namespace tl