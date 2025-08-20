#include "level.hpp"

#include "SDL3/SDL.h"
#include "asset_manager.hpp"
#include "context.hpp"
#include "relationship.hpp"
#include "sprite.hpp"

Level::Level(LevelContentHandle level_content) {
    initByLevelContent(level_content);
}

Level::Level(const Path &filename) {
    auto handle =
            GAME_CONTEXT.m_assets_manager->GetManager<LevelContent>().Load(
                filename);
    initByLevelContent(handle);
}

Level::~Level() {
    for (auto entity: m_entities) {
        RemoveEntity(entity);
    }
    doRemoveEntities();
}

void Level::OnEnter() {
    if (!m_inited) {
        OnInit();
        m_inited = false;
    }
}

void Level::OnInit() {
    for (auto entity: m_entities) {
        auto logic = GAME_CONTEXT.m_entity_logic_manager->Get(entity);
        if (logic) {
            logic->OnInit();
        }
    }
}

void Level::OnLogicUpdate(TimeType time) {
    for (auto entity: m_entities) {
        auto logic = GAME_CONTEXT.m_entity_logic_manager->Get(entity);
        if (logic) {
            logic->OnLogicUpdate(time);
        }
    }
}

void Level::OnRenderUpdate(TimeType time) {
    for (auto entity: m_entities) {
        auto logic = GAME_CONTEXT.m_entity_logic_manager->Get(entity);
        if (logic) {
            logic->OnRenderUpdate(time);
        }
    }
}

void Level::OnQuit() {
    for (auto entity: m_entities) {
        auto logic = GAME_CONTEXT.m_entity_logic_manager->Get(entity);
        if (logic) {
            logic->OnQuit();
        }
    }
}

void Level::PoseUpdate() {
    doRemoveEntities();
}

bool Level::IsInited() const {
    return m_inited;
}

Entity Level::Instantiate(PrefabHandle prefab) {
    Entity entity = GAME_CONTEXT.CreateEntity();
    registerEntity({entity, prefab->m_transform.value_or(Transform{}), prefab});
    m_entities.insert(entity);
    return entity;
}

void Level::RemoveEntity(Entity entity) {
    m_pending_delete_entities.push_back(entity);
}

Entity Level::GetRootEntity() const {
    return m_root_entity;
}

#ifdef TL_ENABLE_EDITOR
void Level::ReloadEntitiesFromPrefab(PrefabHandle prefab) {
    auto it = m_prefab_entity_map.find(prefab);
    if (it == m_prefab_entity_map.end()) {
        return;
    }

    for (auto entity: it->second) {
        auto transform = GAME_CONTEXT.m_transform_manager->Get(entity);

        EntityInstance instance;
        instance.m_entity = entity;
        if (transform) {
            instance.m_transform = *transform;
        }
        instance.m_prefab = prefab;
        RemoveEntity(entity);
        registerEntity(instance);
    }
}

void Level::ReloadEntitiesFromPrefab(UUID uuid) {
    auto prefab =
            GAME_CONTEXT.m_assets_manager->GetManager<Prefab>().Find(uuid);
    if (!prefab) {
        return;
    }
    ReloadEntitiesFromPrefab(prefab);
}
#endif

void Level::initRootEntity() {
    m_root_entity = GAME_CONTEXT.CreateEntity();
    GAME_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    GAME_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity);
}

void Level::registerEntity(const EntityInstance &instance) {
    createEntityByPrefab(
        instance.m_entity,
        instance.m_transform ? &instance.m_transform.value() : nullptr,
        *instance.m_prefab);

#ifdef TL_ENABLE_EDITOR
    m_prefab_entity_map[instance.m_prefab].push_back(instance.m_entity);
#endif
}

void Level::createEntityByPrefab(Entity entity, const Transform *transform,
                                 const Prefab &prefab) {
    if (prefab.m_sprite) {
        GAME_CONTEXT.m_sprite_manager->ReplaceComponent(
            entity, prefab.m_sprite.value());
    }
    if (transform || prefab.m_transform) {
        GAME_CONTEXT.m_transform_manager->ReplaceComponent(
            entity, transform ? *transform : prefab.m_transform.value());
    }
    if (prefab.m_relationship) {
        GAME_CONTEXT.m_relationship_manager->ReplaceComponent(
            entity, prefab.m_relationship.value());
    }
    if (prefab.m_tilemap) {
        GAME_CONTEXT.m_tilemap_component_manager->ReplaceComponent(
            entity, {entity, prefab.m_tilemap.value()});
    }
    if (prefab.m_animation) {
        GAME_CONTEXT.m_animation_player_manager->RegisterEntity(
            entity, prefab.m_animation.value());
    }
    if (prefab.m_cct) {
        GAME_CONTEXT.m_cct_manager->RegisterEntity(entity, entity,
                                                   prefab.m_cct.value());
        GAME_CONTEXT.m_cct_manager->Get(entity)->Teleport(
            prefab.m_transform->m_position);
    }
    if (prefab.m_trigger) {
        GAME_CONTEXT.m_trigger_component_manager->RegisterEntity(
            entity, entity, prefab.m_trigger.value());
    }
    if (prefab.m_type == EntityType::Player) {
        GAME_CONTEXT.m_entity_logic_manager
                ->RegisterEntityByDerive<PlayerLogic>(entity, entity);
    }
}

void Level::initByLevelContent(LevelContentHandle level_content) {
    initRootEntity();

    std::unordered_map<Entity, bool> is_entity_root_map;
    for (auto &instance: level_content->m_entities) {
        is_entity_root_map.emplace(instance.m_entity, true);
    }

    for (auto &instance: level_content->m_entities) {
        if (!instance.m_prefab) {
            LOGW("prefab {} invalid", *instance.m_prefab.GetFilename());
            continue;
        }
        registerEntity(instance);
        if (instance.m_prefab->m_relationship &&
            !instance.m_prefab->m_relationship->m_children.empty()) {
            for (auto &child: instance.m_prefab->m_relationship->m_children) {
                is_entity_root_map[child] = false;
            }
        }
        m_entities.insert(instance.m_entity);
    }

    auto relationship =
            GAME_CONTEXT.m_relationship_manager->Get(GetRootEntity());
    for (auto &[entity, is_root]: is_entity_root_map) {
        relationship->m_children.emplace_back(entity);
    }
}

void Level::doRemoveEntities() {
    std::vector<PrefabHandle> remove_prefabs;

    for (auto entity: m_pending_delete_entities) {
        GAME_CONTEXT.m_sprite_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_transform_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_relationship_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_tilemap_component_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_animation_player_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_cct_manager->RemoveEntity(entity);

        m_entities.erase(entity);

#ifdef TL_ENABLE_EDITOR
        for (auto &[prefab, entities]: m_prefab_entity_map) {
            entities.erase(
                std::remove(entities.begin(), entities.end(), entity),
                entities.end());
            if (entities.empty()) {
                remove_prefabs.push_back(prefab);
            }
        }
#endif
    }

#ifdef TL_ENABLE_EDITOR
    for (auto prefab: remove_prefabs) {
        m_prefab_entity_map.erase(prefab);
    }
#endif

    m_pending_delete_entities.clear();
}

AssetManagerBase<Level>::HandleType LevelManager::Load(const Path &filename,
                                                       bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Level>(filename));
}

void LevelManager::Switch(LevelHandle level) {
    if (m_level) {
        m_level->OnQuit();
    }
    if (level) {
        level->OnEnter();
    }
    m_level = level;
}

void LevelManager::UpdateLogic(TimeType t) {
    if (m_level) {
        m_level->OnLogicUpdate(t);
    }
}

void LevelManager::UpdateRender(TimeType t) {
    if (m_level) {
        m_level->OnRenderUpdate(t);
    }
}

void LevelManager::PoseUpdate() {
    if (!m_level) {
        return;
    }
    m_level->PoseUpdate();
}

LevelHandle LevelManager::GetCurrentLevel() const {
    return m_level;
}

void PlayerLogic::OnInit() {
    auto &animation_manager =
            GAME_CONTEXT.m_assets_manager->GetManager<Animation>();

    m_walk_left = animation_manager.Load(
        "assets/gpa/anim/character/waggo/status_walk_left.animation.xml");
    m_walk_right = animation_manager.Load(
        "assets/gpa/anim/character/waggo/status_walk_right.animation.xml");
    m_walk_up = animation_manager.Load(
        "assets/gpa/anim/character/waggo/status_walk_up.animation.xml");
    m_walk_down = animation_manager.Load(
        "assets/gpa/anim/character/waggo/status_walk_down.animation.xml");
    m_image_sheet = GAME_CONTEXT.m_assets_manager->GetManager<Image>().Load(
        "assets/Characters/Statue/SpriteSheet.png");

    m_gamepad_event_listener =
            GAME_CONTEXT.m_event_system->AddListener<SDL_GamepadDeviceEvent>(
                [&](EventListenerID id, const SDL_GamepadDeviceEvent &event) {
                    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                        if (m_gamepad_id == 0) {
                            m_gamepad_id = event.which;
                        }
                    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                        if (m_gamepad_id == event.which) {
                            m_gamepad_id = 0;
                        }
                    }
                });

    GAME_CONTEXT.m_event_system->AddListener<TriggerEnterEvent>(
        [](EventListenerID, const TriggerEnterEvent &) { LOGI("entered"); });

    GAME_CONTEXT.m_event_system->AddListener<TriggerLeaveEvent>(
        [](EventListenerID, const TriggerLeaveEvent) { LOGI("leave"); });

    GAME_CONTEXT.m_event_system->AddListener<TriggerTouchEvent>(
        [](EventListenerID, const TriggerTouchEvent) { LOGI("touch"); });

    // NOTE: when under android, device will change window size after few frames
    // and send multiple SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event after rotate
    // screen. So we must listen this event and change our button position
    m_window_resize_event_listener =
            GAME_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
                [&](EventListenerID id, const SDL_WindowEvent &event) {
                    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                        int w = event.data1;
                        int h = event.data2;
                        m_touch_joystick.m_circle.m_radius = 100;
                        m_touch_joystick.m_circle.m_center.x = 300;
                        m_touch_joystick.m_circle.m_center.y = h - 300;

                        m_finger_attack_button.m_radius = 50;
                        m_finger_attack_button.m_center.x = w - 200;
                        m_finger_attack_button.m_center.y = h - 200;
                    }
                });
}

void PlayerLogic::OnLogicUpdate(TimeType elapse_time) {
    handleFingerTouchJoystick();

    Entity entity = GetEntity();

    Transform *transform = GAME_CONTEXT.m_transform_manager->Get(entity);

    WalkDirection old_direction = m_walk_direction;

    Vec2 axises = GAME_CONTEXT.m_input_manager->MakeAxises("MoveX", "MoveY")
            .Value(m_gamepad_id);

    auto &action = GAME_CONTEXT.m_input_manager->GetAction("Attack");
    if (action.IsPressed()) {
        transform->m_rotation += 10;
    }

    constexpr float speed = 100;
    auto cct = GAME_CONTEXT.m_cct_manager->Get(entity);
    if (cct) {
        cct->MoveAndSlide(speed * elapse_time * axises);
        transform->m_position = cct->GetPosition();
    }

    AnimationPlayer *player =
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

    GAME_CONTEXT.m_camera.MoveTo(transform->m_position);

    // draw touch joystick
#ifdef SDL_PLATFORM_ANDROID
    GAME_CONTEXT.m_debug_drawer->DrawCircle(m_touch_joystick.m_circle,
                                            Color::Green, elapse_time, false);
    GAME_CONTEXT.m_debug_drawer->DrawCircle(m_finger_attack_button,
                                            Color::Purple, elapse_time, false);

    if (m_move_finger_idx) {
        auto& finger =
            GAME_CONTEXT.m_touches->GetFingers()[m_move_finger_idx.value()];
        auto position =
            finger.Position() * GAME_CONTEXT.m_window->GetWindowSize();
        GAME_CONTEXT.m_debug_drawer->DrawCircle({position, 5}, Color::Red,
                                                elapse_time, false);
    }
#endif
}

void PlayerLogic::OnQuit() {
    GAME_CONTEXT.m_event_system->RemoveListener<SDL_GamepadDeviceEvent>(
        m_gamepad_event_listener);
    GAME_CONTEXT.m_event_system->RemoveListener<SDL_WindowEvent>(
        m_window_resize_event_listener);
}

void PlayerLogic::handleFingerTouchJoystick() {
    auto &fingers = GAME_CONTEXT.m_touches->GetFingers();
    for (size_t i = 0; i < fingers.size(); i++) {
        auto &finger = fingers[i];

        Vec2 position =
                finger.Position() * GAME_CONTEXT.m_window->GetWindowSize();

        if (finger.IsPressed()) {
            if (IsPointInCircle(position, m_touch_joystick.m_circle)) {
                m_move_finger_idx = i;
            }
        } else if (m_move_finger_idx && m_move_finger_idx.value() == i &&
                   finger.IsReleased()) {
            m_move_finger_idx = std::nullopt;
        }

        if (finger.IsPressed()) {
            if (IsPointInCircle(position, m_finger_attack_button)) {
                m_attack_finger_idx = i;
            }
        } else if (m_attack_finger_idx && m_attack_finger_idx.value() == i &&
                   finger.IsReleased()) {
            m_attack_finger_idx = std::nullopt;
        }
    }

    if (!m_move_finger_idx) {
        GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveX", 0);
        GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveY", 0);
    } else {
        auto &finger = fingers[m_move_finger_idx.value()];
        Vec2 position =
                finger.Position() * GAME_CONTEXT.m_window->GetWindowSize();
        Vec2 dir = position - m_touch_joystick.m_circle.m_center;
        float len =
                Clamp(dir.Length(), 0.0f, m_touch_joystick.m_circle.m_radius) /
                m_touch_joystick.m_circle.m_radius;
        dir = dir.Normalize() * len;
        GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveX", dir.x);
        GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveY", dir.y);
    }

    if (!m_attack_finger_idx) {
        GAME_CONTEXT.m_input_manager->AcceptFingerButton(
            "Attack", Action::State::Releasing);
    } else {
        auto &finger = fingers[m_attack_finger_idx.value()];
        if (finger.IsPressed()) {
            GAME_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Pressed);
        } else if (finger.IsPressing()) {
            GAME_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Pressing);
        } else if (finger.IsReleased()) {
            GAME_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Released);
        } else if (finger.IsReleasing()) {
            GAME_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Releasing);
        }
    }
}
