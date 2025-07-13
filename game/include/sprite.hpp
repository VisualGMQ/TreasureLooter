#pragma once
#include "flag.hpp"
#include "image.hpp"
#include "manager.hpp"
#include "renderer.hpp"

struct Sprite {
    Image* m_image{};
    Region m_region;
    Vec2 m_size;
    Flags<Flip> m_flip = Flip::None;

    operator bool() const {
        return m_image;
    }
};

class SpriteManager : public ComponentManager<Sprite> {
public:
    void Update();
};
