#pragma once
#include "pch.hpp"
#include "animation.hpp"
#include "math.hpp"
#include "sprite.hpp"
#include "transform.hpp"

namespace tl {

class GameObjectID {
public:
    friend class GameObjectManager;

    using underlying_type = uint32_t;

    GameObjectID() : id_{InvalidGOID} {}

    operator bool() const { return id_ != InvalidGOID; }

private:
    underlying_type id_;
    static const underlying_type InvalidGOID = 0;

    GameObjectID(underlying_type id) : id_{id} {}
};

class GameObject {
public:
    friend class Context;

    std::string name = "<no-name>";
    Transform transform;
    Sprite sprite;
    Animator animator;

    std::vector<GameObjectID> children;

    const Transform& GetGlobalTransform() const { return globalTransform_; }

private:
    Transform globalTransform_;
};

class GameObjectManager {
public:
    struct CreateResult {
        GameObjectID id;
        GameObject* go = nullptr;

        operator bool() const { return id && go; }
    };

    GameObjectManager();

    CreateResult Create();
    void Destroy(GameObjectID);
    GameObject* Find(GameObjectID);
    void Clear();

    GameObjectID GetRootGOID() const { return rootGO_; }

    GameObject* GetRootGO() { return Find(rootGO_); }

    auto& GetAllGO() const { return goMap_; }

private:
    std::unordered_map<GameObjectID::underlying_type, GameObject> goMap_;
    GameObjectID::underlying_type curID_ = 0;
    GameObjectID rootGO_;
};

}  // namespace tl
