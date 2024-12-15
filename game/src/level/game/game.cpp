#include "level/game/game.hpp"
#include "context.hpp"
#include "level/game/play_controller.hpp"

namespace tl {

GameLevel::GameLevel(Scene& scene)
    : Level{scene},
      bulletPool_{scene},
      animPool_{scene},
      controller_{scene, bulletPool_, animPool_} {}

void GameLevel::Enter() {
    auto& goMgr = GetScene().GetGOMgr();

    GameObject* go = goMgr.Find("game/player");
    TL_RETURN_IF_FALSE(go);
    playerGOID_ = go->GetID();
    controller_.SetPlayer(playerGOID_);

    registTriggerCallback();
}

void GameLevel::Update() {
    controller_.Update();
    bulletPool_.Update();
    animPool_.Update();
}

void GameLevel::Quit() {}

void GameLevel::registTriggerCallback() {
    GetScene().GetEventMgr().RegistCallback(
        Event::Type::EnterTriggerArea,
        [this](const Event& e) {
            auto& enterArea = e.enterTriggerArea;
            TL_RETURN_IF_FALSE(enterArea.go->GetID() == playerGOID_);
            GameObject* player = GetScene().GetGOMgr().Find(playerGOID_);
            TL_RETURN_IF_FALSE(player);

            if (enterArea.area.tile->name == "ninja_scroll") {
                auto& ctx = Context::GetInst();
                player->role = ctx.roleConfigMgr->Find("ninja");
                player->sprite.SetTexture(*player->role.roleTexture);

                animPool_.Create(player->GetGlobalTransform().position,
                                 "game/effect/smoke", 0, true);
            }
        },
        true, "player-change-type");
}

}  // namespace tl
