#pragma once
#include "animation_player.hpp"
#include "cct.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "level.hpp"
#include "schema/gameplay_config.hpp"
#include "sprite.hpp"
#include "state_machine.hpp"
#include "ui.hpp"

/*
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
    SpriteDefinition* GetSprite() const;
    Entity GetWeaponEntity() const;

    void Attack();
    
    ImageHandle m_faceset_image;
    ImageHandle m_sprite_sheet;
    CharacterDirection m_direction = CharacterDirection::Down;
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
    SpriteDefinition* m_sprite{};
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
*/

struct VirtualJoystick {
    Entity m_ui_entity = null_entity;
    Circle m_circle;
    float m_max_drag_dist = 400;
    EventListenerID m_drag_event{};
    EventListenerID m_release_event{};
};

struct VirtualButton {
    Entity m_ui_entity = null_entity;
    EventListenerID m_press_event{};
    const Button* m_button{};
    std::string m_action_name;

    void Update();
};

/*
class PlayerMotorContext : public CharacterMotorContext {
public:
    using CharacterMotorContext::CharacterMotorContext;

    void Initialize(MotorConfigHandle) override;
    void Update(TimeType) override;
    ~PlayerMotorContext() override;

private:
    SDL_JoystickID m_gamepad_id = 0;

    // finger input related
    std::optional<size_t> m_move_finger_idx;
    std::optional<size_t> m_attack_finger_idx;
    EventListenerID m_gamepad_event_listener;
    EventListenerID m_window_resize_event_listener;

    VirtualJoystick m_virtual_joystick;
    VirtualButton m_virtual_attack_button;

    void initVirualJoystick(LevelHandle level);
    void initVirualAttackButton(LevelHandle level);

    void handleJoystickDragEvent(EventListenerID id, const UIDragEvent&);
    void handleJoystickReleaseEvent(EventListenerID id, const UIMouseUpEvent&);
    void handleVirtualAttackButtonPressedEvent(EventListenerID id, const UIMouseDownEvent&);
    void handleVirtualAttackButtonReleasedEvent(EventListenerID id, const UIMouseUpEvent&);
};
*/

class GameplayConfigManager: public ComponentManager<GameplayConfig> {
public:
    void Update(TimeType) {}
};
