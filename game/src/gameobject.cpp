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

}  // namespace tl
