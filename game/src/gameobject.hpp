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

    static GameObjectID Null;

    GameObjectID(const GameObjectID&) = default;
    GameObjectID() : id_{Invalid} {}

    operator bool() const { return id_ != Invalid; }

    GameObjectID& operator=(const GameObjectID&) = default;

    bool operator==(const GameObjectID& o) const {
        return id_ == o.id_;
    }

    bool operator!=(const GameObjectID& o) const {
        return !(*this == o);
    }

    explicit operator underlying_type() const {
        return id_;
    }

private:
    underlying_type id_;

    static const underlying_type Invalid = 0;

    GameObjectID(underlying_type id) : id_{id} {}
};

class GameObject {
public:
    friend class Context;
    friend class GameObjectManager;

    std::string name = "<no-name>";
    Transform transform;
    Sprite sprite;
    Animator animator;

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
    GameObjectManager();

    GameObject* Create();
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
