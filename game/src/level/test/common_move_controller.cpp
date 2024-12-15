#include "level/test/play_controller.hpp"

#include "context.hpp"

namespace tl {

void CommonMoveController::SetMovePlayer(GameObjectID id) {
    id_ = id;
}

void CommonMoveController::Update() {
    GameObject* go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(id_);
    TL_RETURN_IF_FALSE(go);

    auto controller = Context::GetInst().controllerMgr->GetController();
    go->Move(0.3 * controller->GetAxis()); 
}

}
