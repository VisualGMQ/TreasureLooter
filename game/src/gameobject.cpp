#include "gameobject.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"
#include "math.hpp"

namespace tl {

void GameObject::RemoveChild(GameObject& go) {
    auto it = std::remove(children_.begin(), children_.end(), go.GetID());
    if (it == children_.end()) {
        return;
    }

    go.parent_ = GameObjectID{};
    children_.erase(it, children_.end());
}

void GameObject::AppendChild(GameObject& go) {
    children_.push_back(go.GetID());
    go.parent_ = id_;
}

void GameObject::SetChildToNext(GameObject& target, GameObject& go) {
    auto it = std::find(children_.begin(), children_.end(), target.GetID());
    if (it == children_.end()) {
        return;
    }

    target.parent_ = id_;

    children_.insert(it + 1, go.GetID());
}

void GameObject::SetChildToPrev(GameObject& target, GameObject& go) {
    auto it = std::find(children_.begin(), children_.end(), target.GetID());
    if (it == children_.end()) {
        return;
    }

    target.parent_ = id_;

    children_.insert(it, go.GetID());
}

void GameObject::InsertChild(GameObject& go, size_t idx) {
    go.parent_ = id_;

    children_.insert(children_.begin() + idx, go.GetID());
}

GameObject* GameObjectManager::Create() {
    auto result = goMap_.emplace(++curID_, GameObject{});
    if (result.second) {
        result.first->second.id_ = result.first->first;
        return &result.first->second;
    }
    return nullptr;
}

GameObjectID::underlying_type GameObjectManager::curID_ = 0;

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

GameObject* GameObjectManager::Clone(GameObject& src) {
    GameObject* go = Create();

    go->name = "<no-name>";
    go->transform = src.transform;
    go->tilemap = src.tilemap;
    go->physicActor = src.physicActor;
    go->animator = src.animator;

    go->sprite.SetRegion(src.sprite.GetRegion());
    if (go->sprite.IsTexture() && src.sprite.GetTexture()) {
        go->sprite.SetTexture(*src.sprite.GetTexture());
    } else if (go->sprite.IsText() && go->sprite.GetFont()) {
        go->sprite.SetFontTexture(FontTexture{*go->sprite.GetFont(),
                                              go->sprite.GetText(),
                                              go->sprite.GetFontSize()});
    }
    go->sprite.anchor = src.sprite.anchor;
    go->sprite.color = src.sprite.color;
    go->sprite.flip = src.sprite.flip;
    go->sprite.isEnable = src.sprite.isEnable;

    return go;
}

GameObject* GameObjectManager::Clone(GameObjectID id) {
    TL_RETURN_NULL_IF_FALSE(id);

    GameObject* src = Find(id);
    TL_RETURN_NULL_IF_FALSE(src);

    return Clone(*src);
}

}  // namespace tl
