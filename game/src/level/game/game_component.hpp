#pragma once
#include "role_config.hpp"

namespace tl {

enum class GameObjectType {
    Unknown,
    Role,
    Chest,
    Wall,
};

struct Carry {
    GameObjectID other;
};

struct GameComponent {
    GameObjectType type = GameObjectType::Unknown;
    RoleConfig role;
    Carry carry;
    GameObjectID stickGOID;
};

bool CanCarry(const GameComponent&);

}