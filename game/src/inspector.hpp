#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class Inspector {
public:
    void Update();

private:
    std::string lastGOName_;

    void updateName(const std::string&) const;
    void updateTransform(const std::string& title, Transform&);
    void updateTransform(const std::string& title, const Transform&);
    void updateSprite(Sprite&);
    void updateAnimator(Animator&);
    void updateTileMap(TileMap&);
    void updateTransformGeneric(Transform&);
    void updateTransformGeneric(const Transform& transform);
    void updatePhysicActor(PhysicActor&);
};

}
