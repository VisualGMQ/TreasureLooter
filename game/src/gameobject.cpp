#include "gameobject.hpp"
#include "log.hpp"
#include "math.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl {

GameObjectID GameObjectID::Null;

void GameObject::RemoveChild(GameObjectID go) {
    auto it = std::remove(children_.begin(), children_.end(), go);
    if (it == children_.end()) {
        return;
    }
    GameObject* removedGO = Context::GetInst().goMgr->Find(go);
    TL_RETURN_IF(removedGO);

    removedGO->parent_ = GameObjectID::Null;
    children_.erase(it, children_.end());
}

void GameObject::AppendChild(GameObjectID go) {
    GameObject* appendGO = Context::GetInst().goMgr->Find(go);
    if (!appendGO) {
        return;
    }
    children_.push_back(go);
    appendGO->parent_ = id_;
}

void GameObject::SetChildToNext(GameObjectID target, GameObjectID go) {
    auto it = std::find(children_.begin(), children_.end(), target);
    if (it == children_.end()) {
        return;
    }

    GameObject* insertGO = Context::GetInst().goMgr->Find(go);
    TL_RETURN_IF(insertGO);

    insertGO->parent_ = id_;

    children_.insert(it + 1, go);
}

void GameObject::SetChildToPrev(GameObjectID target, GameObjectID go) {
    auto it = std::find(children_.begin(), children_.end(), target);
    if (it == children_.end()) {
        return;
    }

    GameObject* insertGO = Context::GetInst().goMgr->Find(go);
    TL_RETURN_IF(insertGO);
    insertGO->parent_ = id_;

    children_.insert(it, go);
}

void GameObject::InsertChild(GameObjectID go, size_t idx) {
    GameObject* insertGO = Context::GetInst().goMgr->Find(go);
    TL_RETURN_IF(insertGO);
    insertGO->parent_ = id_;

    children_.insert(children_.begin() + idx, go);
}

GameObject* GameObjectManager::Create() {
    auto result = goMap_.emplace(++curID_, GameObject{});
    if (result.second) {
        result.first->second.id_ = result.first->first;
        return &result.first->second;
    }
    return nullptr;
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

GameObject* GameObjectManager::Find(std::string_view name) {
    for (auto& [id, go] : goMap_) {
        if (go.name == name) {
            return &go;
        }
    }
    return nullptr;
}

void GameObjectManager::Clear() {
    goMap_.clear();
}

}  // namespace tl
