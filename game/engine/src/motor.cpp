#include "engine/motor.hpp"

#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/enemy_state.hpp"
#include "engine/relationship.hpp"

CharacterMotorContext::CharacterMotorContext(Entity entity)
    : m_entity{entity} {}

void CharacterMotorContext::Initialize(MotorConfigHandle config) {
    m_transform = CURRENT_CONTEXT.m_transform_manager->Get(m_entity);
    m_move_animation =
        CURRENT_CONTEXT.m_animation_player_manager->Get(m_entity);
    m_cct = CURRENT_CONTEXT.m_cct_manager->Get(m_entity);
    m_sprite = CURRENT_CONTEXT.m_sprite_manager->Get(m_entity);
    m_faceset_image = config->m_faceset;
    m_sprite_sheet = config->m_sprite_sheet;
    m_direction = CharacterDirection::Down;
    m_move_speed = config->m_speed;
    m_move_left_animation = config->m_move_left_animation;
    m_move_right_animation = config->m_move_right_animation;
    m_move_up_animation = config->m_move_up_animation;
    m_move_down_animation = config->m_move_down_animation;
    m_target_position = m_transform->m_position;

    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(m_entity);
    if (relationship) {
        if (config->m_weapon_entity) {
            if (config->m_weapon_entity.value() <
                relationship->m_children.size()) {
                m_weapon_entity =
                    relationship->m_children[config->m_weapon_entity.value()];
            }
        }
    }
}

void CharacterMotorContext::Move(const Vec2& dir, TimeType duration) {
    if (dir == Vec2{}) {
        if (m_move_animation) {
            m_move_animation->Stop();
        }
    } else {
        if (auto weapon_anim = CURRENT_CONTEXT.m_animation_player_manager->Get(
                m_weapon_entity);
            weapon_anim && !weapon_anim->IsPlaying()) {
            m_weapon_dir = dir.Normalize();
        }
    }

    CharacterDirection old_direction = m_direction;

    if (dir.x < 0) {
        m_direction = CharacterDirection::Left;
    }
    if (dir.x > 0) {
        m_direction = CharacterDirection::Right;
    }
    if (dir.y < 0) {
        m_direction = CharacterDirection::Up;
    }
    if (dir.y > 0) {
        m_direction = CharacterDirection::Down;
    }

    if (m_cct) {
        m_cct->MoveAndSlide(dir * m_move_speed * duration);
        m_transform->m_position = m_cct->GetPosition();
    }

    if ((!m_move_animation->IsPlaying() && dir != Vec2::ZERO) ||
        old_direction != m_direction) {
        switch (m_direction) {
            case CharacterDirection::Up:
                m_move_animation->ChangeAnimation(m_move_up_animation);
                break;
            case CharacterDirection::Left:
                m_move_animation->ChangeAnimation(m_move_left_animation);
                break;
            case CharacterDirection::Right:
                m_move_animation->ChangeAnimation(m_move_right_animation);
                break;
            case CharacterDirection::Down:
                m_move_animation->ChangeAnimation(m_move_down_animation);
                break;
        }
        m_move_animation->Play();
    }

    if (dir == Vec2::ZERO) {
        switch (m_direction) {
            case CharacterDirection::Up:
                m_sprite->m_region.m_topleft = {16, 0};
                break;
            case CharacterDirection::Left:
                m_sprite->m_region.m_topleft = {32, 0};
                break;
            case CharacterDirection::Right:
                m_sprite->m_region.m_topleft = {48, 0};
                break;
            case CharacterDirection::Down:
                m_sprite->m_region.m_topleft = {0, 0};
                break;
        }
    }
}

void CharacterMotorContext::Teleport(const Vec2& p) {
    if (m_cct) {
        m_cct->Teleport(p);
    }
}

void CharacterMotorContext::Update(TimeType) {
    if (auto weapon_anim =
            CURRENT_CONTEXT.m_animation_player_manager->Get(m_weapon_entity);
        weapon_anim && !weapon_anim->IsPlaying()) {
        auto transform =
            CURRENT_CONTEXT.m_transform_manager->Get(m_weapon_entity);
        transform->m_rotation = GetAngle(m_weapon_dir, Vec2::X_UNIT);
    }
}

Entity CharacterMotorContext::GetEntity() const {
    return m_entity;
}

float CharacterMotorContext::GetSpeed() const {
    return m_move_speed;
}

const Vec2& CharacterMotorContext::GetPosition() const {
    return m_transform->m_position;
}

AnimationPlayer* CharacterMotorContext::GetAnimationPlayer() {
    return m_move_animation;
}

CharacterDirection CharacterMotorContext::GetDirection() const {
    return m_direction;
}

Sprite* CharacterMotorContext::GetSprite() const {
    return m_sprite;
}

Entity CharacterMotorContext::GetWeaponEntity() const {
    return m_weapon_entity;
}

void CharacterMotorContext::Attack() {
    if (m_weapon_entity == null_entity) {
        return;
    }

    auto weapon_attack_animator =
        CURRENT_CONTEXT.m_animation_player_manager->Get(m_weapon_entity);
    auto weapon_transform =
        CURRENT_CONTEXT.m_transform_manager->Get(m_weapon_entity);
    if (weapon_attack_animator && !weapon_attack_animator->IsPlaying()) {
        weapon_attack_animator->Stop();
        weapon_attack_animator->Play();
    }
    if (weapon_transform) {
        Radians angle = GetAngle(m_weapon_dir, Vec2::X_UNIT);
        weapon_transform->m_rotation = angle;
    }
}

void EnemyMotorContext::Initialize(MotorConfigHandle config) {
    CharacterMotorContext::Initialize(config);

    m_state_machine.ChangePayload(this);
    m_state_machine.ChangeState(
        &StateSingletonManager::GetState<EnemyIdleState>());
}

void EnemyMotorContext::Update(TimeType time) {
    CharacterMotorContext::Update(time);

    m_state_machine.OnUpdate();
}

StateMachine<EnemyMotorContext>& EnemyMotorContext::GetStateMachine() {
    return m_state_machine;
}

void VirtualButton::Update() {
    if (!m_button) {
        return;
    }

    if (m_button->IsPressing()) {
        GAME_CONTEXT.m_input_manager->AcceptFingerButton(
            m_action_name, Action::State::Pressing);
    }

    if (m_button->IsReleasing()) {
        GAME_CONTEXT.m_input_manager->AcceptFingerButton(
            m_action_name, Action::State::Releasing);
        m_button = nullptr;
    }
}

void PlayerMotorContext::Initialize(MotorConfigHandle motor_config) {
    CharacterMotorContext::Initialize(motor_config);

    const GameConfig& game_config = GAME_CONTEXT.GetGameConfig();

    m_virtual_joystick.m_circle.m_radius =
        game_config.m_virtual_joystick.m_radius;
    m_virtual_joystick.m_circle.m_center.y =
        GAME_CONTEXT.GetGameConfig().m_logic_size.h -
        game_config.m_virtual_joystick.m_offset.y;
    m_virtual_joystick.m_circle.m_center.x =
        game_config.m_virtual_joystick.m_offset.x;
    m_virtual_joystick.m_max_drag_dist =
        game_config.m_virtual_joystick.m_max_drag_dist;

#ifdef TL_ANDROID
    LevelHandle level = GAME_CONTEXT.m_level_manager->GetCurrentLevel();
    if (level) {
        initVirualJoystick(level);
        initVirualAttackButton(level);

        // NOTE: when under android, device will change window size after few
        // frames and send multiple SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event
        // after rotate screen. So we must listen this event and change our
        // button position
        m_window_resize_event_listener =
            CURRENT_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
                [&](EventListenerID, const SDL_WindowEvent& event) {
                    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                        int h = event.data2;
                        m_virtual_joystick.m_circle.m_center.x =
                            game_config.m_virtual_joystick.m_offset.x;
                        m_virtual_joystick.m_circle.m_center.y =
                            h - game_config.m_virtual_joystick.m_offset.y;
                    }
                });
    }
#endif

    CURRENT_CONTEXT.m_event_system->AddListener<TriggerEnterEvent>(
        [](EventListenerID, const TriggerEnterEvent&) { LOGI("entered"); });

    CURRENT_CONTEXT.m_event_system->AddListener<TriggerLeaveEvent>(
        [](EventListenerID, const TriggerLeaveEvent&) { LOGI("leave"); });
    CURRENT_CONTEXT.m_event_system->AddListener<TriggerTouchEvent>(
        [](EventListenerID, const TriggerTouchEvent&) { LOGI("touch"); });

    m_gamepad_event_listener =
        CURRENT_CONTEXT.m_event_system->AddListener<SDL_GamepadDeviceEvent>(
            [&](EventListenerID, const SDL_GamepadDeviceEvent& event) {
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
}

void PlayerMotorContext::Update(TimeType duration) {
    CharacterMotorContext::Update(duration);

    Entity entity = GetEntity();

    Transform* transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);

    Vec2 axises = CURRENT_CONTEXT.m_input_manager->MakeAxises("MoveX", "MoveY")
                      .Value(m_gamepad_id);

    auto& action = CURRENT_CONTEXT.m_input_manager->GetAction("Attack");
    if (action.IsPressed()) {
        Attack();
    }

    Move(axises, duration);

    CURRENT_CONTEXT.m_camera.MoveTo(transform->m_position);

    m_virtual_attack_button.Update();
}

PlayerMotorContext::~PlayerMotorContext() {
    GAME_CONTEXT.m_event_system->RemoveListener<UIDragEvent>(
        m_virtual_joystick.m_drag_event);
    GAME_CONTEXT.m_event_system->RemoveListener<UIMouseUpEvent>(
        m_virtual_joystick.m_release_event);
}

void PlayerMotorContext::initVirualJoystick(LevelHandle level) {
    PrefabHandle joystick_prefab =
        GAME_CONTEXT.m_assets_manager->GetManager<Prefab>().Load(
            "assets/gpa/ui/android_joystick.prefab.xml");
    Transform transform;
    transform.m_size.w = m_virtual_joystick.m_circle.m_radius * 2.0;
    transform.m_size.h = m_virtual_joystick.m_circle.m_radius * 2.0;
    transform.m_position = m_virtual_joystick.m_circle.m_center -
                           Vec2{m_virtual_joystick.m_circle.m_radius,
                                m_virtual_joystick.m_circle.m_radius};
    Entity entity = level->Instantiate(joystick_prefab);
    *GAME_CONTEXT.m_transform_manager->Get(entity) = transform;

    Relationship* ui_relationship =
        GAME_CONTEXT.m_relationship_manager->Get(level->GetUIRootEntity());
    ui_relationship->m_children.push_back(entity);
    m_virtual_joystick.m_ui_entity = entity;

    GAME_CONTEXT.m_event_system->AddListener<UIDragEvent>(
        std::bind(&PlayerMotorContext::handleJoystickDragEvent, this,
                  std::placeholders::_1, std::placeholders::_2));
    GAME_CONTEXT.m_event_system->AddListener<UIMouseUpEvent>(
        std::bind(&PlayerMotorContext::handleJoystickReleaseEvent, this,
                  std::placeholders::_1, std::placeholders::_2));
}

void PlayerMotorContext::initVirualAttackButton(LevelHandle level) {
    PrefabHandle button_prefab =
        GAME_CONTEXT.m_assets_manager->GetManager<Prefab>().Load(
            "assets/gpa/ui/attack_button.prefab.xml");
    Transform transform;
    auto& game_config = GAME_CONTEXT.GetGameConfig();
    transform.m_size.w = game_config.m_virtual_joystick.m_radius * 2.0;
    transform.m_size.h = game_config.m_virtual_joystick.m_radius * 2.0;
    transform.m_position = GAME_CONTEXT.m_window->GetWindowSize() -
                           game_config.m_virtual_attack_button.m_offset;
    Entity entity = level->Instantiate(button_prefab);
    *GAME_CONTEXT.m_transform_manager->Get(entity) = transform;

    Relationship* ui_relationship =
        GAME_CONTEXT.m_relationship_manager->Get(level->GetUIRootEntity());
    ui_relationship->m_children.push_back(entity);
    m_virtual_attack_button.m_ui_entity = entity;
    m_virtual_attack_button.m_action_name = "Attack";

    GAME_CONTEXT.m_event_system->AddListener<UIMouseDownEvent>(
        std::bind(&PlayerMotorContext::handleVirtualAttackButtonPressedEvent,
                  this, std::placeholders::_1, std::placeholders::_2));
    GAME_CONTEXT.m_event_system->AddListener<UIMouseUpEvent>(
        std::bind(&PlayerMotorContext::handleVirtualAttackButtonReleasedEvent,
                  this, std::placeholders::_1, std::placeholders::_2));
}

void PlayerMotorContext::handleJoystickDragEvent(EventListenerID id,
                                                 const UIDragEvent& event) {
    if (event.m_entity != m_virtual_joystick.m_ui_entity) {
        return;
    }

    Transform* transform =
        GAME_CONTEXT.m_transform_manager->Get(event.m_entity);
    Vec2 position = GAME_CONTEXT.m_mouse->Position();
    Vec2 dir = position - m_virtual_joystick.m_circle.m_center;
    float dir_len = dir.Length();
    Vec2 offset = Vec2::ZERO;
    float offset_len = 0;
    if (!FLT_EQ(dir_len, 0)) {
        dir /= dir_len;
        offset = dir;
    }
    offset_len = std::min(dir_len, m_virtual_joystick.m_max_drag_dist);
    transform->m_position = m_virtual_joystick.m_circle.m_center +
                            offset * offset_len -
                            Vec2{m_virtual_joystick.m_circle.m_radius,
                                 m_virtual_joystick.m_circle.m_radius};
    GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent(
        "MoveX", dir.x * offset_len / m_virtual_joystick.m_max_drag_dist);
    GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent(
        "MoveY", dir.y * offset_len / m_virtual_joystick.m_max_drag_dist);
}

void PlayerMotorContext::handleJoystickReleaseEvent(
    EventListenerID id, const UIMouseUpEvent& event) {
    if (event.m_entity != m_virtual_joystick.m_ui_entity) {
        return;
    }

    Transform* transform =
        GAME_CONTEXT.m_transform_manager->Get(event.m_entity);
    transform->m_position = m_virtual_joystick.m_circle.m_center -
                            Vec2{m_virtual_joystick.m_circle.m_radius,
                                 m_virtual_joystick.m_circle.m_radius};
    GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveX", 0);
    GAME_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveY", 0);
}

void PlayerMotorContext::handleVirtualAttackButtonPressedEvent(
    EventListenerID id, const UIMouseDownEvent& event) {
    if (event.m_entity != m_virtual_attack_button.m_ui_entity) {
        return;
    }

    m_virtual_attack_button.m_button = &event.m_button;
    GAME_CONTEXT.m_input_manager->AcceptFingerButton(
        m_virtual_attack_button.m_action_name, Action::State::Pressed);
}

void PlayerMotorContext::handleVirtualAttackButtonReleasedEvent(
    EventListenerID id, const UIMouseUpEvent& event) {
    if (event.m_entity != m_virtual_attack_button.m_ui_entity) {
        return;
    }

    m_virtual_attack_button.m_button = &event.m_button;
    GAME_CONTEXT.m_input_manager->AcceptFingerButton(
        m_virtual_attack_button.m_action_name, Action::State::Released);
}

void MotorManager::Update(TimeType duration) {
    for (auto& [_, component] : m_components) {
        if (!component.m_enable) {
            continue;
        }

        component.m_component->Update(duration);
    }
}
