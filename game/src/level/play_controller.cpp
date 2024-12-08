#include "play_controller.hpp"

#include "context.hpp"

namespace tl {

Bullet& BulletPool::Create(const Vec2& pos, float duration, Animation* anim, Vec2 dir) {
    Bullet bullet;
    bullet.duration = duration;
    bullet.dir = dir;

    if (!cache_.empty()) {
        Bullet& cacheBullet = cache_.back();
        bullet.go = cacheBullet.go;
        bullet.go->enable = true;
        bullet.go->animator.animation = anim;
        cache_.pop_back();
        bullets_.push_back(std::move(bullet));
    } else {
        auto& scene = Context::GetInst().sceneMgr->GetCurScene();
        auto& goMgr = scene.GetGOMgr();
        GameObject* go = goMgr.Create();
        go->name = "bullet";
        go->animator.animation = anim;
        go->animator.SetLoop(InfLoop);
        go->transform.scale = Vec2{2, 2};
        go->physicActor.enable = true;
        go->physicActor.shape.SetCircle(Circle{Vec2::ZERO, 8});
        bullet.go = go;
        bullets_.push_back(std::move(bullet));

        scene.GetRootGO()->AppendChild(*go);
    }

    bullets_.back().go->animator.Play();
    bullets_.back().go->transform.position = pos;

    return bullets_.back();
}

void BulletPool::Update() {
    size_t idx = 0;
    while (idx < bullets_.size()) {
        auto& bullet = bullets_[idx];
        bullet.go->Move(bullet.dir);
        if (bullet.duration <= 0) {
            bullet.go->enable = false;
            cache_.push_back(std::move(bullet));
            bullets_.erase(bullets_.begin() + idx);
            continue;
        }
        bullet.duration--;
        idx++;
    }
}

PlayController::PlayController(GameObjectID id) : id_{id}, dir_{0, 1} {}

void PlayController::Update() {
    GameObject* go =
        Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(id_);
    TL_RETURN_IF_FALSE(go && go->role);

    auto controller = Context::GetInst().controllerMgr->GetController();
    auto axis = controller->GetAxis();
    if (axis != Vec2::ZERO) {
        dir_ = axis;
    }
    go->Move(go->role.speed * axis);

    if (go->role.type == RoleConfig::Type::Ninja &&
        controller->GetAttackButton().IsPressed()) {
        bulletPool_.Create(
            go->transform.position + dir_ * 16, 1000,
            Context::GetInst().animMgr->Find("game/weapon/shuriken"), dir_ * 1);
    }

    updateAnimation(axis);
    bulletPool_.Update();
}

void PlayController::updateAnimation(const Vec2& axis) {
    GameObject* go =
        Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(id_);
    TL_RETURN_IF_FALSE(go && go->enable && go->role);

    if (axis == Vec2::ZERO) {
        go->animator.Stop();
        if (dir_.x < 0) {
            go->sprite.SetTexture(*go->role.roleTexture);
            go->sprite.SetRegion(go->role.standLeftRegion);
            go->sprite.flip = Flip::None;
        }

        if (dir_.x > 0) {
            go->sprite.SetTexture(*go->role.roleTexture);
            go->sprite.SetRegion(go->role.standLeftRegion);
            go->sprite.flip = Flip::Horizontal;
        }

        if (dir_.y < 0) {
            go->sprite.SetTexture(*go->role.roleTexture);
            go->sprite.SetRegion(go->role.standUpRegion);
            go->sprite.flip = Flip::None;
        }

        if (dir_.y > 0) {
            go->sprite.SetTexture(*go->role.roleTexture);
            go->sprite.SetRegion(go->role.standDownRegion);
            go->sprite.flip = Flip::None;
        }
        return;
    }

    if (axis.x < 0) {
        go->animator.animation = go->role.walkLeftAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    if (axis.x > 0) {
        go->animator.animation = go->role.walkLeftAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::Horizontal;
    }

    if (axis.y < 0) {
        go->animator.animation = go->role.walkUpAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    if (axis.y > 0) {
        go->animator.animation = go->role.walkDownAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    go->animator.Play();
}

}  // namespace tl
