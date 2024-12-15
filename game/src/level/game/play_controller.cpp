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
    TL_RETURN_IF_FALSE(go && go->game.role);

    auto controller = Context::GetInst().controllerMgr->GetController();
    auto axis = controller->GetAxis();
    if (axis != Vec2::ZERO) {
        dir_ = axis;
    }
    go->Move(go->game.role.speed * axis);

    if (go->game.role.type == RoleConfig::Type::Ninja &&
        controller->GetAttackButton().IsPressed()) {
        bulletPool_.Create(
            go->GetLocalPosition() + dir_ * 16, 100,
            Context::GetInst().animMgr->Find("game/weapon/shuriken"),
            dir_ * 10);
    }

    if (controller->GetInteractButton().IsPressed()) {
        if (go->game.stickGOID) {
            GameObject* stickGO = scene_.GetGOMgr().Find(go->game.stickGOID);
            if (CanCarry(stickGO->game)) {
                go->game.carry.other = go->game.stickGOID;
            }
        } else {
            go->game.carry.other = GameObjectID{};
        }
    }

    updateAnimation(axis);
    bulletPool_.Update();
}

void PlayController::updateAnimation(const Vec2& axis) {
    GameObject* go = scene_.GetGOMgr().Find(id_);
    TL_RETURN_IF_FALSE(go && go->enable && go->game.role);

    const RoleConfig& role = go->game.role;
    
    if (axis == Vec2::ZERO) {
        go->animator.Stop();
        if (dir_.x < 0) {
            go->sprite.SetTexture(*role.roleTexture);
            go->sprite.SetRegion(role.standLeftRegion);
            go->sprite.flip = Flip::None;
        }

        if (dir_.x > 0) {
            go->sprite.SetTexture(*role.roleTexture);
            go->sprite.SetRegion(role.standLeftRegion);
            go->sprite.flip = Flip::Horizontal;
        }

        if (dir_.y < 0) {
            go->sprite.SetTexture(*role.roleTexture);
            go->sprite.SetRegion(role.standUpRegion);
            go->sprite.flip = Flip::None;
        }

        if (dir_.y > 0) {
            go->sprite.SetTexture(*role.roleTexture);
            go->sprite.SetRegion(role.standDownRegion);
            go->sprite.flip = Flip::None;
        }
        return;
    }

    if (axis.x < 0) {
        go->animator.animation = role.walkLeftAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    if (axis.x > 0) {
        go->animator.animation = role.walkLeftAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::Horizontal;
    }

    if (axis.y < 0) {
        go->animator.animation = role.walkUpAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    if (axis.y > 0) {
        go->animator.animation = role.walkDownAnim;
        go->animator.SetLoop(InfLoop);
        go->sprite.flip = Flip::None;
    }

    go->animator.Play();
}

}  // namespace tl
