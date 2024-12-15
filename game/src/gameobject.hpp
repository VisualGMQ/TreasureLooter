#pragma once
#include "role_config.hpp"
#include "animation.hpp"
#include "camera.hpp"
#include "id.hpp"
#include "math.hpp"
#include "pch.hpp"
#include "physics.hpp"
#include "sprite.hpp"
#include "tilemap.hpp"
#include "transform.hpp"

namespace tl {

class GameObject {
public:
    friend class Scene;
    friend class GameObjectManager;
    friend class PhysicsScene;
    friend class Prefab;

    std::string name = "<no-name>";
    Sprite sprite;
    Animator animator;
    TileMap* tilemap = nullptr;
    PhysicActor physicActor;
    Camera camera;
    RoleConfig role;
    bool enable = true;

    const Transform& GetLocalTransform() const;
    void SetLocalTransform(const Transform& transform);
    void SetLocalPosition(const Vec2& position);
    void SetLocalRotation(float rotation);
    void SetLocalScale(const Vec2& scale);
    Vec2 GetLocalPosition() const;
    float GetLocalRotation() const;
    Vec2 GetLocalScale() const;

    const Transform& GetGlobalTransform() const;
    GameObjectID GetID() const { return id_; }
    GameObjectID GetParentID() const { return parent_; }

    auto& GetChildren() const { return children_; }
    void RemoveChild(GameObject&);
    void AppendChild(GameObject&);
    void SetChildToNext(GameObject& target, GameObject& go);
    void SetChildToPrev(GameObject& target, GameObject& go);
    void InsertChild(GameObject& go, size_t idx);

    void Move(const Vec2& offset);

    void UpdateTransform(const Transform& parentTrans, bool syncPhysics);

private:
    Transform localTransform_;
    bool needUpdateTransform_ = true;
    Transform globalTransform_;
    GameObjectID id_;
    GameObjectID parent_;
    std::vector<GameObjectID> children_;
};

class GameObjectManager {
public:
    GameObjectManager() = default;
    GameObjectManager(const GameObjectManager&) = delete;
    GameObjectManager& operator=(const GameObjectManager&) = delete;
    GameObjectManager(GameObjectManager&&) = default;
    GameObjectManager& operator=(GameObjectManager&&) = default;

    GameObject* Create();
    void Destroy(GameObjectID);
    GameObject* Find(GameObjectID);
    const GameObject* Find(GameObjectID) const;
    GameObject* Find(std::string_view);
    const GameObject* Find(std::string_view) const;
    GameObject* Clone(GameObjectID);
    GameObject* Clone(GameObject&);
    void Clear();

    auto& GetAllGO() const { return goMap_; }
    auto& GetAllGO() { return goMap_; }

private:
    std::unordered_map<GameObjectID::underlying_type, GameObject> goMap_;
    static GameObjectID::underlying_type curID_;
};

}  // namespace tl
