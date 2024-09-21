#include "gameobject.hpp"
#include "log.hpp"
#include "math.hpp"

namespace tl {

GameObjectManager::GameObjectManager() {
    auto result = Create();
    if (!result) {
        LOGE("root go create failed");
    }
    rootGO_ = result.id;
}

GameObjectManager::CreateResult GameObjectManager::Create() {
    auto result = goMap_.emplace(++curID_, GameObject{});
    if (result.second) {
        return CreateResult{GameObjectID{result.first->first},
                            &result.first->second};
    }
    return CreateResult{};
}

void GameObjectManager::Destroy(GameObjectID o) {
    goMap_.erase(o.id_);
}

GameObject* GameObjectManager::Find(GameObjectID o) {
    if (auto it = goMap_.find(o.id_); it != goMap_.end()) {
        return &it->second;
    }
    return nullptr;
}

void GameObjectManager::Clear() {
    goMap_.clear();
}

void GameObjectManager::Update() {
    updateGOGlobalTransform(nullptr, GetRootGO());
}

void GameObjectManager::updateGOGlobalTransform(GameObject* parent,
                                                GameObject* go) {
    if (!parent) {
        go->globalTransform_ = go->transform;
    } else {
        go->globalTransform_.scale = go->transform.scale * parent->globalTransform_.scale;
        go->globalTransform_.rotation = go->transform.rotation + parent->globalTransform_.rotation;
        float radians = Deg2Rad(go->transform.rotation);
        float cos = std::cos(radians);
        float sin = std::sin(radians);
        go->globalTransform_.position = Rotate(go->transform.position, go->globalTransform_.rotation);
        go->globalTransform_.position += parent->globalTransform_.position;
    }

    for (GameObjectID id : go->children) {
        GameObject* child = Find(id);
        if (child) {
            updateGOGlobalTransform(go, child);
        }
    }
}

}  // namespace tl
