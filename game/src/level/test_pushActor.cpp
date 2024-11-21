#pragma once
#include "level/test_pushActor.hpp"
#include "context.hpp"

namespace tl {

void TestPushActor::Init() {
    auto& ctx = Context::GetInst();
    auto& goMgr = ctx.sceneMgr->GetCurScene().GetGOMgr();
    auto& scene = ctx.sceneMgr->GetCurScene();
    auto root = scene.GetRootGO();

    auto prefab = goMgr.Find("test/push_actor/rectPrefab");
    TL_RETURN_IF_FALSE(prefab);

    box1_ = goMgr.Clone(*prefab);
    box1_->name = "box1";
    box1_->transform.position = Vec2{400, 100};
    root->AppendChild(*box1_);

    box2_ = goMgr.Clone(*prefab);
    box2_->name = "box2";
    box2_->transform.position = Vec2{400, 141};
    root->AppendChild(*box2_);
}

void TestPushActor::Enter() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = true;
    Context::GetInst().gameController.reset();
}

void TestPushActor::Update() {
    auto& ctx = Context::GetInst();
    auto go = ctx.sceneMgr->GetCurScene().GetGOMgr().Find(
        "test/push_actor/rectPrefab");

    auto axis = ctx.controllerMgr->GetController()->GetAxis();
    if (axis.x != 0 || axis.y != 0) {
        go->physicActor.SetMovement(axis * 0.2);
    }
}

}  // namespace tl