#pragma once
#include "schema/gameplay_config.hpp"
#include "engine/sprite.hpp"
#include "engine/ui.hpp"

class GameplayConfigManager: public ComponentManager<GameplayConfig> {
public:
    void Update(TimeType) {}
};
