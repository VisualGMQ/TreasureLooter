#include "level/test_playground.hpp"
#include "context.hpp"

namespace tl {

void CharacterController::SetCharacter(GameObjectID goID) {
    go_ = goID;
}

GameObject* CharacterController::GetCharacter() {
    return Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(go_);
}

bool CharacterController::HasCharacter() const {
    return Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(go_);
}

void CharacterController::Update() {
    auto& goMgr = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr();
    GameObject* go = goMgr.Find(go_);
    TL_RETURN_IF_FALSE(go);

    const controller::Controller* controller =
        Context::GetInst().controllerMgr->GetController();
    TL_RETURN_IF_FALSE(controller);

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

void TestLevel::Enter() {
    auto controller = std::make_unique<CharacterController>();
    auto go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        "test-controller");
    TL_RETURN_IF_FALSE(go);
    controller->SetCharacter(go->GetID());
    Context::GetInst().gameController = std::move(controller);
}

}  // namespace tl