#include "gameobject.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"
#include "math.hpp"
#include "profile.hpp"

namespace tl {

void GameObject::SetLocalPosition(const Vec2& position) {
    localTransform_.position = position;
    needUpdateTransform_ = true;
}

void GameObject::SetLocalRotation(float rotation) {
    localTransform_.rotation = rotation;
    needUpdateTransform_ = true;
}

void GameObject::SetLocalScale(const Vec2& scale) {
    localTransform_.scale = scale;
    needUpdateTransform_ = true;
}

Vec2 GameObject::GetLocalPosition() const {
    return localTransform_.position;
}

float GameObject::GetLocalRotation() const {
    return localTransform_.rotation;
}

Vec2 GameObject::GetLocalScale() const {
    return localTransform_.scale;
}

const Transform& GameObject::GetGlobalTransform() const {
    return globalTransform_;
}

void GameObject::SetLocalTransform(const Transform& transform) {
    needUpdateTransform_ = true;
    localTransform_ = transform;
}

const Transform& GameObject::GetLocalTransform() const {
    return localTransform_;
}

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

void GameObject::Move(const Vec2& offset) {
    if (physicActor) {
        physicActor.SetMovement(offset);
    } else {
        localTransform_.position += offset;
        needUpdateTransform_ = true;
    }
}

void GameObject::UpdateTransform(const Transform& parentTrans,
                                 bool syncPhysics) {
    PROFILE_FUNC();
    TL_RETURN_IF_FALSE(enable);

    if (syncPhysics && physicActor) {
        Vec2 dir = physicActor.shape.GetCenter() * GetGlobalTransform().scale;
        globalTransform_.position = physicActor.GetCollideShape().GetCenter() -
                                    Rotate(dir, localTransform_.rotation);
        localTransform_ =
            CalcLocalTransformToParent(parentTrans, GetGlobalTransform());
    } else {
        if (needUpdateTransform_) {
            globalTransform_ =
                CalcTransformFromParent(parentTrans, localTransform_);
            if (tilemap) {
                tilemap->UpdateTransform(globalTransform_);
            }
        }
    }

    for (auto c : GetChildren()) {
        GameObject* go =
            Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(c);
        TL_CONTINUE_IF_FALSE(go);

        go->UpdateTransform(GetGlobalTransform(), syncPhysics);
    }

    needUpdateTransform_ = false;
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
    go->localTransform_ = src.localTransform_;
    go->camera = src.camera;
    go->role = src.role;
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
