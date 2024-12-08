#pragma once
#include "common.hpp"
#include "game_controller.hpp"

namespace tl {

class CommonMoveController: public GameController {
public:
    explicit CommonMoveController(GameObjectID id);
    void Update();
    
private:
    GameObjectID id_;
};

}
