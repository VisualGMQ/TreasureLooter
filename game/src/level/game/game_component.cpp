#include "level/game/game_component.hpp"

namespace tl {

bool CanCarry(const GameComponent& game) {
    return game.type == GameObjectType::Chest;
}

}
