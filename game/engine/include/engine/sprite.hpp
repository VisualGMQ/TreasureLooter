#pragma once
#include "flag.hpp"
#include "image.hpp"
#include "manager.hpp"
#include "renderer.hpp"
#include "schema/sprite.hpp"

class SpriteManager : public ComponentManager<Sprite> {
public:
    void Update();
};
