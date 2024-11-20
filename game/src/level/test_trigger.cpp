#include "level/test_trigger.hpp"

#include "level/common_move_controller.hpp"
#include "context.hpp"

namespace tl {

void TestTriggerLevel::Enter() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = true;

    GameObject* go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        "test/trigger_area/playerPrefabe");
    TL_RETURN_IF_FALSE(go);
    Context::GetInst().gameController = std::make_unique<CommonMoveController>(
        go->GetID());

    auto& eventMgr = Context::GetInst().eventMgr;
    eventMgr->RegistCallback(Event::Type::EnterTriggerArea, [&, id = go->GetID()](const Event& e) {
        TL_RETURN_IF_FALSE(id == e.enterTriggerArea.go->GetID());
        e.enterTriggerArea.go->sprite.color = Color::Red;
    }, false, "test-trigger-entred");
    eventMgr->RegistCallback(Event::Type::LeaveTriggerArea, [&, id = go->GetID()](const Event& e) {
        TL_RETURN_IF_FALSE(id == e.leaveTriggerArea.go->GetID());
        e.leaveTriggerArea.go->sprite.color = Color::White;
        Context::GetInst().eventMgr->RemoveCallback(Event::Type::LeaveTriggerArea, "test-trigger-leave");
    }, false, "test-trigger-leave");
}

}
