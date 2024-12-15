#include "play_controller.hpp"

#include "collision_group.hpp"
#include "context.hpp"

namespace tl {

PlayController::PlayController(Scene& scene, BulletPool& bulletPool,
                               AnimPool& animPool)
    : scene_{scene}, bulletPool_{bulletPool}, animPool_{animPool}, dir_{0, 1} {}

void PlayController::SetPlayer(GameObjectID id) {
    id_ = id;
}

void PlayController::Update() {
    GameObject* go = scene_.GetGOMgr().Find(id_);
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
            go->GetLocalPosition() + dir_ * 16, 100,
            Context::GetInst().animMgr->Find("game/weapon/shuriken"),
            dir_ * 10);
    }

    updateAnimation(axis);
    bulletPool_.Update();
}

void PlayController::updateAnimation(const Vec2& axis) {
    GameObject* go = scene_.GetGOMgr().Find(id_);
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
