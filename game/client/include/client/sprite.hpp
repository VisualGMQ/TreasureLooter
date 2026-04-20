#pragma once
#include "common/flag.hpp"
#include "common/image.hpp"
#include "common/manager.hpp"
#include "client/renderer.hpp"
#include "schema/sprite.hpp"

using Sprite = SpriteDefinition;

class SpriteManager : public ComponentManager<Sprite> {
public:
    void SubmitDrawCommand(Entity);
};
