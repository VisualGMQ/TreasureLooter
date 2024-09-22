#pragma once
#include "math.hpp"
#include "pch.hpp"

namespace tl {

struct Transform {
    Vec2 position;
    Vec2 scale = Vec2::ONES;
    float rotation = 0;  // in degrees
};

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
    friend class GameObjectManager;

    Transform transform;

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
    void Update();

    GameObjectID GetRootGOID() const { return rootGO_; }

    GameObject* GetRootGO() { return Find(rootGO_); }

    auto& GetAllGO() const { return goMap_; }

private:
    std::unordered_map<GameObjectID::underlying_type, GameObject> goMap_;
    GameObjectID::underlying_type curID_ = 0;
    GameObjectID rootGO_;

    void updateGOGlobalTransform(GameObject* parent, GameObject* go);
};

}  // namespace tl
