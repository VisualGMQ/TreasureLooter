#pragma once
#include "common.hpp"

namespace tl {

class CommonMoveController {
public:
    void SetMovePlayer(GameObjectID id);
    void Update();
    
private:
    GameObjectID id_;
};

}
