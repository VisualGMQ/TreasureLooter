#pragma once
#include "flags.hpp"

namespace tl {

enum class CollisionGroup {
    Pawn = 1,
    Weapon,
};

using CollisionGroupFlags = Flags<CollisionGroup>;

}
