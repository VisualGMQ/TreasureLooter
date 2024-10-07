#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class GameController {
public:
    void SetCharacter(GameObjectID);
    GameObject* GetCharacter();
    bool HasCharacter() const;
    void Update();

private:
    GameObjectID go_;
};

};  // namespace tl