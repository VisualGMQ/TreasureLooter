#include "engine/motor.hpp"
#include "engine/context.hpp"
#include "engine/enemy_state.hpp"
#include "engine/relationship.hpp"
#include "enet.h"

CharacterMotorContext::CharacterMotorContext(Entity entity)
    : m_entity{entity} {}

void CharacterMotorContext::Initialize(MotorConfigHandle config) {
    m_transform = CURRENT_CONTEXT.m_transform_manager->Get(m_entity);
    m_move_animation = CURRENT_CONTEXT.m_animation_player_manager->Get(m_entity);
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
        if (auto weapon_anim =
                CURRENT_CONTEXT.m_animation_player_manager->Get(m_weapon_entity);
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
        auto transform = CURRENT_CONTEXT.m_transform_manager->Get(m_weapon_entity);
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

void PlayerMotorContext::Initialize(MotorConfigHandle motor_config) {
    CharacterMotorContext::Initialize(motor_config);

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

    // NOTE: when under android, device will change window size after few frames
    // and send multiple SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event after rotate
    // screen. So we must listen this event and change our button position
    m_window_resize_event_listener =
        CURRENT_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
            [&](EventListenerID, const SDL_WindowEvent& event) {
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

void PlayerMotorContext::Update(TimeType duration) {
    CharacterMotorContext::Update(duration);

    handleFingerTouchJoystick();

    Entity entity = GetEntity();

    Transform* transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);

    Vec2 axises = CURRENT_CONTEXT.m_input_manager->MakeAxises("MoveX", "MoveY")
                      .Value(m_gamepad_id);

    auto& action = CURRENT_CONTEXT.m_input_manager->GetAction("Attack");
    if (action.IsPressed()) {
        Attack();

        constexpr std::string_view msg = "HelloEnet";
        ENetPacket* packet = enet_packet_create(msg.data(), msg.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(GAME_CONTEXT.m_enet_peer, 0, packet);
    }

    Move(axises, duration);

    CURRENT_CONTEXT.m_camera.MoveTo(transform->m_position);

    // draw touch joystick
#ifdef SDL_PLATFORM_ANDROID
    CURRENT_CONTEXT.m_debug_drawer->DrawCircle(m_touch_joystick.m_circle,
                                            Color::Green, duration, false);
    CURRENT_CONTEXT.m_debug_drawer->DrawCircle(m_finger_attack_button,
                                            Color::Purple, duration, false);

    if (m_move_finger_idx) {
        auto& finger =
            CURRENT_CONTEXT.m_touches->GetFingers()[m_move_finger_idx.value()];
        auto position =
            finger.Position() * CURRENT_CONTEXT.m_window->GetWindowSize();
        CURRENT_CONTEXT.m_debug_drawer->DrawCircle({position, 5}, Color::Red,
                                                duration, false);
    }
#endif
}

void PlayerMotorContext::handleFingerTouchJoystick() {
    auto& fingers = CURRENT_CONTEXT.m_touches->GetFingers();
    for (size_t i = 0; i < fingers.size(); i++) {
        auto& finger = fingers[i];

        Vec2 position =
            finger.Position() * CURRENT_CONTEXT.m_window->GetWindowSize();

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
        CURRENT_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveX", 0);
        CURRENT_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveY", 0);
    } else {
        auto& finger = fingers[m_move_finger_idx.value()];
        Vec2 position =
            finger.Position() * CURRENT_CONTEXT.m_window->GetWindowSize();
        Vec2 dir = position - m_touch_joystick.m_circle.m_center;
        float len =
            Clamp(dir.Length(), 0.0f, m_touch_joystick.m_circle.m_radius) /
            m_touch_joystick.m_circle.m_radius;
        dir = dir.Normalize() * len;
        CURRENT_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveX", dir.x);
        CURRENT_CONTEXT.m_input_manager->AcceptFingerAxisEvent("MoveY", dir.y);
    }

    if (!m_attack_finger_idx) {
        CURRENT_CONTEXT.m_input_manager->AcceptFingerButton(
            "Attack", Action::State::Releasing);
    } else {
        auto& finger = fingers[m_attack_finger_idx.value()];
        if (finger.IsPressed()) {
            CURRENT_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Pressed);
        } else if (finger.IsPressing()) {
            CURRENT_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Pressing);
        } else if (finger.IsReleased()) {
            CURRENT_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Released);
        } else if (finger.IsReleasing()) {
            CURRENT_CONTEXT.m_input_manager->AcceptFingerButton(
                "Attack", Action::State::Releasing);
        }
    }
}

void MotorManager::Update(TimeType duration) {
    for (auto& [_, component] : m_components) {
        if (!component.m_enable) {
            continue;
        }

        component.m_component->Update(duration);
    }
}
