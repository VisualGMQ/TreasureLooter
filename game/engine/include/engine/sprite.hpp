#pragma once
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/manager.hpp"
#include "engine/renderer.hpp"
#include "schema/sprite.hpp"

using Sprite = SpriteDefinition;

class SpriteManager : public ComponentManager<Sprite> {
public:
    void Update();
};
