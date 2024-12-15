#include "level/game/game.hpp"
#include "context.hpp"
#include "level/game/play_controller.hpp"

namespace tl {

GameLevel::GameLevel(Scene& scene)
    : Level{scene},
      bulletPool_{scene},
      animPool_{scene},
      controller_{scene, bulletPool_, animPool_} {}

void GameLevel::Init() {
    for (auto&& [id, go] : GetScene().GetGOMgr().GetAllGO()) {
        if (go.game.type == GameObjectType::Role) {
            roles_.push_back(id);
        }
    }
    
    registTriggerCallback();
    registStickCallback();
}

void GameLevel::Enter() {
    auto& goMgr = GetScene().GetGOMgr();

    GameObject* go = goMgr.Find("game/player");
    TL_RETURN_IF_FALSE(go);
    playerGOID_ = go->GetID();
    controller_.SetPlayer(playerGOID_);
}

void GameLevel::Update() {
    controller_.Update();
    bulletPool_.Update();
    animPool_.Update();
    updateCarray();
    clearStickState();
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
                player->game.role = ctx.roleConfigMgr->Find("ninja");
                player->sprite.SetTexture(*player->game.role.roleTexture);

                animPool_.Create(player->GetGlobalTransform().position,
                                 "game/effect/smoke", 0, true);
            }
        },
        true, "player-change-type");
}

void GameLevel::registStickCallback() {
    GetScene().GetEventMgr().RegistCallback(
        Event::Type::Collision,
        [this](const Event& e) {
            auto& collision = e.collision;
            if (collision.src.go->game.type == GameObjectType::Role) {
                collision.src.go->game.stickGOID = collision.dst.go->GetID();
            }

             if (collision.dst.go->game.type == GameObjectType::Role) {
                 collision.dst.go->game.stickGOID = collision.src.go->GetID();
             }
         },
         false, "role-stick");   
}

void GameLevel::updateCarray() {
    for (auto& id : roles_) {
        GameObject* go = GetScene().GetGOMgr().Find(id);
        GameObject* other = GetScene().GetGOMgr().Find(go->game.carry.other);
        TL_CONTINUE_IF_FALSE(go && other);
        other->SetLocalPosition(go->GetLocalPosition());
    }
}

void GameLevel::clearStickState() {
    for (auto& role : roles_) {
        GameObject* go = GetScene().GetGOMgr().Find(role);
        go->game.stickGOID = GameObjectID{};
    }
}

}  // namespace tl
