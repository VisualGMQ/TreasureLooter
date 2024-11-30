#include "game.hpp"
#include "context.hpp"
#include "play_controller.hpp"

namespace tl {


void GameLevel::Enter() {
    auto& ctx = Context::GetInst();
    auto& goMgr = ctx.sceneMgr->GetCurScene().GetGOMgr();
    {
        GameObject* go = goMgr.Find("player");
        TL_RETURN_IF_FALSE(go);
        playerGOID_ = go->GetID();
        ctx.gameController = std::make_unique<PlayController>(playerGOID_);
    }

    {
        GameObject* go = goMgr.Find("effect");
        TL_RETURN_IF_FALSE(go);
        effectGOID_ = go->GetID();
    }

    registTriggerCallback();
}

void GameLevel::Update() {
    auto& goMgr = GetScene().GetGOMgr();
    GameObject* go = goMgr.Find(effectGOID_);
    TL_RETURN_IF_FALSE(go);

    if (!go->animator.IsPlaying()) {
        go->enable = false;
    }
}

void GameLevel::Quit() {
    unregistTriggerCallback();
}

void GameLevel::registTriggerCallback() {
    Context::GetInst().eventMgr->RegistCallback(
        Event::Type::EnterTriggerArea,
        [level = this](const Event& e) {
            auto& enterArea = e.enterTriggerArea;
            TL_RETURN_IF_FALSE(enterArea.go->GetID() == level->playerGOID_);
            GameObject* player =
                level->GetScene().GetGOMgr().Find(level->playerGOID_);
            TL_RETURN_IF_FALSE(player);

            if (enterArea.area.tile->name == "ninja_scroll") {
                auto& ctx = Context::GetInst();
                auto texture = ctx.textureMgr->Find("Ninja.png");
                TL_RETURN_IF_FALSE(texture);
                player->role = ctx.roleConfigMgr->Find("ninja");
                player->sprite.SetTexture(*player->role.roleTexture);

                auto effectGO =
                    level->GetScene().GetGOMgr().Find(level->effectGOID_);
                effectGO->enable = true;
                effectGO->animator.Play();
            }
        },
        false, "player-change-type");
}

void GameLevel::unregistTriggerCallback() {
    Context::GetInst().eventMgr->RemoveCallback(Event::Type::EnterTriggerArea,
                                                "player-change-type");
}

}  // namespace tl
