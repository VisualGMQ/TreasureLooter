#include "level/game/anim_pool.hpp"

#include "context.hpp"
#include "scene.hpp"

namespace tl {

AnimPool::AnimPool(Scene& scene) : scene_{scene} {
    auto& goMgr = scene_.GetGOMgr();
    rootGO_ = goMgr.Create();

    rootGO_->name = "animations";

    GameObject* root = scene.GetRootGO();
    root->AppendChild(*rootGO_);
}

Anim& AnimPool::Create(const Vec2& pos, const std::string& animName, int loop,
                       bool hideWhenFinish) {
    Anim& anim = getUnusedAnim();
    anim.hideWhenFinish = hideWhenFinish;
    anim.go->SetLocalPosition(pos);
    anim.go->SetLocalScale(Vec2{2, 2});
    anim.go->enable = true;
    anim.go->sprite.enable = true;
    anim.go->animator.animation = Context::GetInst().animMgr->Find(animName);
    anim.go->animator.Play();
    anim.go->animator.SetLoop(loop);
    return anim;
}

void AnimPool::Update() {
    for (auto& [id, anim] : anims_) {
        TL_CONTINUE_IF_FALSE(anim.go->enable);

        if (!anim.go->animator.IsPlaying() && anim.hideWhenFinish) {
            anim.go->enable = false;
            cache_.push_back(id);
        }
    }
}

Anim& AnimPool::getUnusedAnim() {
    if (cache_.empty()) {
        auto& goMgr = scene_.GetGOMgr();
        GameObject* go = goMgr.Create();
        go->name = "anim";
        rootGO_->AppendChild(*go);
        Anim anim;
        anim.go = go;
        return anims_.emplace(go->GetID(), std::move(anim)).first->second;
    }
    
    GameObjectID id = cache_.back();
    cache_.pop_back();
    return anims_[id];
}

}  // namespace tl
