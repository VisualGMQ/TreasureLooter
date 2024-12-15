#include "level/test/test_trigger.hpp"

#include "context.hpp"
#include "level/test/play_controller.hpp"

namespace tl {

void TestTriggerLevel::Enter() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = true;

    GameObject* go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        "test/trigger_area/playerPrefabe");
    TL_RETURN_IF_FALSE(go);
    controller_.SetMovePlayer(go->GetID());

    auto& eventMgr = Context::GetInst().GetCurScene().GetEventMgr();
    eventMgr.RegistCallback(
        Event::Type::EnterTriggerArea,
        [&, id = go->GetID()](const Event& e) {
            TL_RETURN_IF_FALSE(id == e.enterTriggerArea.go->GetID());
            if (e.enterTriggerArea.area.go->GetID() ==
                Context::GetInst()
                    .sceneMgr->GetCurScene()
                    .GetGOMgr()
                    .Find("test/trigger_area/strike")
                    ->GetID()) {
                e.enterTriggerArea.go->sprite.color = Color::Red;
            } else if (e.enterTriggerArea.area.go->GetID() ==
                       Context::GetInst()
                           .sceneMgr->GetCurScene()
                           .GetGOMgr()
                           .Find("test/trigger_area/cold")
                           ->GetID()) {
                e.enterTriggerArea.go->sprite.color = Color::Blue;
            }
        },
        false, "test-trigger-entred");
    eventMgr.RegistCallback(
        Event::Type::LeaveTriggerArea,
        [&, id = go->GetID()](const Event& e) {
            TL_RETURN_IF_FALSE(id == e.leaveTriggerArea.go->GetID());
            e.leaveTriggerArea.go->sprite.color = Color::White;
        },
        false, "test-trigger-leave");
}

void TestTriggerLevel::Update() {
    controller_.Update();
}

}  // namespace tl
