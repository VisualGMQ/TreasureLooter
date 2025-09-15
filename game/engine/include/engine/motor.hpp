#pragma once
#include "animation_player.hpp"
#include "cct.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "schema/motor_config.hpp"
#include "sprite.hpp"
#include "state_machine.hpp"

enum class CharacterDirection {
    Left,
    Right,
    Down,
    Up,
};

class MotorContext {
public:
    virtual ~MotorContext() = default;

    virtual void Initialize(MotorConfigHandle) = 0;
    virtual void Update(TimeType) = 0;
};

class CharacterMotorContext : public MotorContext {
public:
    explicit CharacterMotorContext(Entity entity);

    void Initialize(MotorConfigHandle) override;

    void Move(const Vec2& dir, TimeType duration);

    void Teleport(const Vec2& p);

    void Update(TimeType) override;

    Entity GetEntity() const;

    float GetSpeed() const;
    const Vec2& GetPosition() const;

    Vec2 m_target_position;

    AnimationPlayer* GetAnimationPlayer();
    CharacterDirection GetDirection() const;
    Sprite* GetSprite() const;
    Entity GetWeaponEntity() const;

    void Attack();
    
    ImageHandle m_faceset_image;
    ImageHandle m_sprite_sheet;
    CharacterDirection m_direction;
    AnimationHandle m_move_left_animation;
    AnimationHandle m_move_right_animation;
    AnimationHandle m_move_up_animation;
    AnimationHandle m_move_down_animation;
    float m_move_speed{20};

protected:
    Entity m_entity = null_entity;
    Entity m_weapon_entity = null_entity;
    
    Transform* m_transform{};
    AnimationPlayer* m_move_animation{};
    CharacterController* m_cct{};
    Sprite* m_sprite{};
    Vec2 m_weapon_dir = Vec2::X_UNIT;
};

class EnemyMotorContext : public CharacterMotorContext {
public:
    using CharacterMotorContext::CharacterMotorContext;

    void Initialize(MotorConfigHandle) override;

    void Update(TimeType) override;

    StateMachine<EnemyMotorContext>& GetStateMachine();

    TimeType m_idle_time{3};
    TimeType m_cur_idle_time{};
    TimeType m_force_idle_time{10};
    TimeType m_cur_force_idle_time{0};

private:
    StateMachine<EnemyMotorContext> m_state_machine;
};

class PlayerMotorContext : public CharacterMotorContext {
public:
    using CharacterMotorContext::CharacterMotorContext;

    void Initialize(MotorConfigHandle) override;
    void Update(TimeType) override;

private:
    struct TouchJoystick {
        Circle m_circle;
    };

    SDL_JoystickID m_gamepad_id = 0;

    // finger input related
    TouchJoystick m_touch_joystick;
    Circle m_finger_attack_button;
    std::optional<size_t> m_move_finger_idx;
    std::optional<size_t> m_attack_finger_idx;
    EventListenerID m_gamepad_event_listener;
    EventListenerID m_window_resize_event_listener;

    void handleFingerTouchJoystick();
};

class MotorManager : public ComponentManager<MotorContext> {
public:
    void Update(TimeType);
};
