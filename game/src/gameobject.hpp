#pragma once
#include "pch.hpp"
#include "animation.hpp"
#include "math.hpp"
#include "sprite.hpp"
#include "transform.hpp"
#include "tilemap.hpp"
#include "id.hpp"
#include "physics.hpp"

namespace tl {

class GameObject;
class GameObjectManager;

using GameObjectID = ID<GameObject, GameObjectManager>;

class GameObject {
public:
    friend class Scene;
    friend class GameObjectManager;

    std::string name = "<no-name>";
    Transform transform;
    Sprite sprite;
    Animator animator;
    TileMap* tilemap = nullptr;
    PhysicsActor physicsActor;

    const Transform& GetGlobalTransform() const { return globalTransform_; }
    GameObjectID GetID() const { return id_; }
    GameObjectID GetParentID() const { return parent_; }

    auto& GetChildren() const { return children_; }
    void RemoveChild(GameObjectID);
    void AppendChild(GameObjectID);
    void SetChildToNext(GameObjectID target, GameObjectID go);
    void SetChildToPrev(GameObjectID target, GameObjectID go);
    void InsertChild(GameObjectID go, size_t idx);

private:
    Transform globalTransform_;
    GameObjectID id_;
    GameObjectID parent_;
    std::vector<GameObjectID> children_;
};

class GameObjectManager {
public:
    GameObject* Create();
    void Destroy(GameObjectID);
    GameObject* Find(GameObjectID);
    GameObject* Find(std::string_view);
    void Clear();

    auto& GetAllGO() const { return goMap_; }

private:
    std::unordered_map<GameObjectID::underlying_type, GameObject> goMap_;
    GameObjectID::underlying_type curID_ = 0;
};

}  // namespace tl
