#include "level/game/bullet.hpp"

#include "collision_group.hpp"
#include "context.hpp"

namespace tl {

BulletPool::BulletPool(Scene& scene) : scene_{scene} {
    auto& goMgr = scene.GetGOMgr();
    bulletRootGO_ = goMgr.Create();

    bulletRootGO_->name = "bullets";

    GameObject* root = scene.GetRootGO();
    root->AppendChild(*bulletRootGO_);
}

Bullet& BulletPool::Create(const Vec2& pos, float duration, Animation* anim, Vec2 dir) {
    Bullet* bullet = nullptr;
    if (cache_.empty()) {
        auto& goMgr = scene_.GetGOMgr();
        GameObject* go = goMgr.Create();
        go->name = "bullet";
        go->animator.animation = anim;
        go->animator.SetLoop(InfLoop);
        go->SetLocalScale(Vec2{2, 2});
        go->physicActor.enable = true;
        go->physicActor.shape.SetCircle(Circle{Vec2::ZERO, 8});
        go->physicActor.filter = CollisionGroupFlags{CollisionGroup::Pawn};
        bullet = &bullets_.emplace(go->GetID(), Bullet{}).first->second;
        bullet->go = go;
        Context::GetInst().GetCurScene().GetEventMgr().RegistCallback(
            Event::Type::Collision, [this](const Event& event) {
                GameObject* srcGO = event.collision.src.go;
                GameObject* dstGO = event.collision.dst.go;

                auto it1 = this->bullets_.find(srcGO->GetID());
                if (it1 != this->bullets_.end()) {
                    it1->second.duration = 0;
                }
                
                auto it2 = this->bullets_.find(dstGO->GetID());
                if (it2 != this->bullets_.end()) {
                    it2->second.duration = 0;
                }
        });
        bulletRootGO_->AppendChild(*go);
    } else {
        GameObjectID id = cache_.back();
        bullet = &bullets_[id];
        bullet->go->enable = true;
        cache_.pop_back();
    }

    bullet->duration = duration;
    bullet->dir = dir;
    bullet->go->animator.Play();
    bullet->go->SetLocalPosition(pos);

    return *bullet;
}

void BulletPool::Update() {
    for (auto& [id, bullet] : bullets_) {
        TL_CONTINUE_IF_FALSE(bullet.go->enable);
        
        if (bullet.duration <= 0) {
            bullet.go->enable = false;
            cache_.push_back(id);
        } else {
            bullet.go->Move(bullet.dir);
            bullet.duration--;
        }
    }
}

}
