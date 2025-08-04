#include "level.hpp"

#include "SDL3/SDL.h"
#include "asset_manager.hpp"
#include "context.hpp"
#include "physics.hpp"
#include "relationship.hpp"
#include "sprite.hpp"

Level::Level() {
    initRootEntity();
}

Entity Level::GetRootEntity() const {
    return m_root_entity;
}

void Level::initRootEntity() {
    m_root_entity = GAME_CONTEXT.CreateEntity();
    GAME_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    GAME_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity);
}

void GameLevel::OnInit() {
    auto& animation_manager =
        GAME_CONTEXT.m_assets_manager->GetManager<AnimationHandle>();

    m_walk_left =
        animation_manager.Load("assets/gpa/status_walk_left.animation.xml");
    m_walk_right =
        animation_manager.Load("assets/gpa/status_walk_right.animation.xml");
    m_walk_up =
        animation_manager.Load("assets/gpa/status_walk_up.animation.xml");
    m_walk_down =
        animation_manager.Load("assets/gpa/status_walk_down.animation.xml");
    m_image_sheet =
        GAME_CONTEXT.m_assets_manager->GetManager<ImageHandle>().Load(
            "assets/Characters/Statue/SpriteSheet.png");

    {
        auto result = LoadAsset<EntityInstance>("assets/gpa/waggo.prefab.xml");
        result.m_payload.m_entity = GAME_CONTEXT.CreateEntity();
        GAME_CONTEXT.RegisterEntity(result.m_payload);

        auto root_relationship =
            GAME_CONTEXT.m_relationship_manager->Get(GetRootEntity());
        root_relationship->m_children.push_back(result.m_payload.m_entity);
        m_player_entity = result.m_payload.m_entity;

        auto cct = GAME_CONTEXT.m_cct_manager->Get(result.m_payload.m_entity);
        auto transform =
            GAME_CONTEXT.m_transform_manager->Get(result.m_payload.m_entity);
        cct->Teleport(transform->m_position);
    }
    {
        auto result =
            LoadAsset<EntityInstance>("assets/gpa/tilemap.prefab.xml");
        result.m_payload.m_entity = GAME_CONTEXT.CreateEntity();
        GAME_CONTEXT.RegisterEntity(result.m_payload);

        auto root_relationship =
            GAME_CONTEXT.m_relationship_manager->Get(GetRootEntity());
        root_relationship->m_children.push_back(result.m_payload.m_entity);
    }
}

void GameLevel::OnLogicUpdate(TimeType elapse_time) {
    auto children = GAME_CONTEXT.m_relationship_manager->Get(GetRootEntity());
    Entity entity = children->m_children[0];

    Transform* transform = GAME_CONTEXT.m_transform_manager->Get(entity);

    WalkDirection old_direction = m_walk_direction;

    Vec2 axises =
        GAME_CONTEXT.m_input_manager->MakeAxises("MoveX", "MoveY").Value();

    constexpr float speed = 500;
    auto cct = GAME_CONTEXT.m_cct_manager->Get(entity);
    if (cct) {
        cct->MoveAndSlide(speed * elapse_time * axises);
        transform->m_position = cct->GetPosition();
    }

    AnimationPlayer* player =
        GAME_CONTEXT.m_animation_player_manager->Get(entity);

    if (axises == Vec2{}) {
        player->Stop();
    }
    if (axises.y < 0) {
        m_walk_direction = WalkDirection::Up;
    }
    if (axises.y > 0) {
        m_walk_direction = WalkDirection::Down;
    }
    if (axises.x < 0) {
        m_walk_direction = WalkDirection::Left;
    }
    if (axises.x > 0) {
        m_walk_direction = WalkDirection::Right;
    }

    if ((!player->IsPlaying() && axises != Vec2::ZERO) ||
        old_direction != m_walk_direction) {
        switch (m_walk_direction) {
            case WalkDirection::Up:
                player->ChangeAnimation(m_walk_up);
                break;
            case WalkDirection::Left:
                player->ChangeAnimation(m_walk_left);
                break;
            case WalkDirection::Right:
                player->ChangeAnimation(m_walk_right);
                break;
            case WalkDirection::Down:
                player->ChangeAnimation(m_walk_down);
                break;
        }
        player->Play();
    }

    if (axises == Vec2::ZERO) {
        auto sprite = GAME_CONTEXT.m_sprite_manager->Get(entity);
        switch (m_walk_direction) {
            case WalkDirection::Up:
                sprite->m_region.m_topleft = {16, 0};
                break;
            case WalkDirection::Left:
                sprite->m_region.m_topleft = {32, 0};
                break;
            case WalkDirection::Right:
                sprite->m_region.m_topleft = {48, 0};
                break;
            case WalkDirection::Down:
                sprite->m_region.m_topleft = {0, 0};
                break;
        }
    }
}

void GameLevel::OnQuit() {}