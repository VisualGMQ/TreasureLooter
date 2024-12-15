#pragma once
#include "3rdlib/tinyxml2/tinyxml2.h"
#include "animation.hpp"
#include "level/game/game_component.hpp"
#include "role_config.hpp"
#include "sprite.hpp"
#include "tilemap.hpp"

#include <string>

namespace tl {

class GameObject;
class GameObjectManager;

class GameObjectParser {
public:
    explicit GameObjectParser(GameObjectManager& mgr);
    GameObject* operator()(const tinyxml2::XMLElement& node);

private:
    GameObjectManager& mgr_;
    
    GameObject* parseGORecurse(const tinyxml2::XMLElement& node);
    GameObject* parseGO(const tinyxml2::XMLElement& node);
    Sprite parseSprite(const tinyxml2::XMLElement& elem) const;
    Transform parseTransform(const tinyxml2::XMLElement& elem) const;
    TileMap* parseTileMap(const tinyxml2::XMLElement& elem) const;
    PhysicActor parsePhysicActor(const tinyxml2::XMLElement& elem) const;
    Animator parseAnimator(const tinyxml2::XMLElement& elem) const;
    Camera parseCamera(const tinyxml2::XMLElement& elem) const;
    GameComponent parseGameComponent(const tinyxml2::XMLElement& elem) const;
    GameObjectType parseGameObjectType(const tinyxml2::XMLElement& elem) const;
    const RoleConfig& parseRole(const tinyxml2::XMLElement& elem) const;
};

}
