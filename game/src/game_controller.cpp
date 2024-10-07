#include "game_controller.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl {

void GameController::SetCharacter(GameObjectID goID) {
    go_ = goID;
}

GameObject* GameController::GetCharacter() {
    return Context::GetInst().goMgr->Find(go_);
}

bool GameController::HasCharacter() const {
    return Context::GetInst().goMgr->Find(go_);
}

void GameController::Update() {
    TL_RETURN_IF(Context::GetInst().sceneMgr->GetCurScene()->HasGO(go_));

    auto& goMgr = Context::GetInst().goMgr;
    GameObject* go = goMgr->Find(go_);
    TL_RETURN_IF(go);

    const controller::Controller* controller =
        Context::GetInst().controllerMgr->GetController();
    TL_RETURN_IF(controller);

    Vec2 axis = controller->GetAxis();
    constexpr float Speed = 0.1;

    if (axis.LengthSqrd() > 0.01) {
        go->transform.position += Speed * axis;
    }

    if (controller->GetAttackButton().IsPressed()) {
        go->transform.rotation += 5;
    }
    if (controller->GetDefendButton().IsPressing()) {
        go->transform.rotation -= 0.1;
    }
    if (controller->GetInteractButton().IsPressed()) {
        go->transform.rotation = 0;
    }
}

}  // namespace tl

