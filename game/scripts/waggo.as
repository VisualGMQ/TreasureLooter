#include "behavior.as"
#include "generic_forward_weapon.as"

enum CharacterDirection {
    Left,
    Right,
    Down,
    Up,
}

class Waggo: TL::Behavior {
    Waggo(TL::Entity entity) {
        super(entity);
    }

    void OnInit() override {
        @m_anim_player = GetAnimationPlayerComponent();
        @m_cct = GetCCTComponent();
        @m_transform = GetTransformComponent();
        @m_sprite = GetSpriteComponent();

        TL::GameplayConfig@ config = GetGameplayConfigComponent();
        m_image_sheet = config.m_sprite_sheet;
        m_move_speed = config.m_speed;
        m_move_left_anim = config.m_move_left_animation;
        m_move_right_anim = config.m_move_right_animation;
        m_move_up_anim = config.m_move_up_animation;
        m_move_down_anim = config.m_move_down_animation;

        TL::Relationship@ relationship = GetRelationshipComponent();
        if (relationship !is null && !relationship.m_children.isEmpty()) {
            if (config.m_weapon_entity.HasValue()) {
                uint32 idx = config.m_weapon_entity.Value();
                if (idx < relationship.m_children.length()) {
                    m_weapon_entity = relationship.m_children[idx];
                }
            }
        }

        if (!m_weapon_entity.IsNull()) {
            TL::Behavior@ behavior = GetBehavior(m_weapon_entity);
            @m_weapon_script = cast<GenericForwardWeapon>(@behavior);

            if (m_weapon_script is null) {
                TL::Log("can't get weapon script");
            }
        }
    }

    void OnUpdate(TL::TimeType delta_time) override {
		TL::AnimationPlayer@ weapon_anim = GetAnimationPlayerComponentFrom(m_weapon_entity);

        TL::Vec2 axises = TL::GetGameContext().m_input_manager.MakeAxises("MoveX", "MoveY")
                                    // TODO: use gamepad ID
                                    .Value(0);
        TL::Action@ action = TL::GetGameContext().m_input_manager.GetAction("Attack");
        if (action.IsPressed(0)) {
            attack();
        }

        move(axises, delta_time);

        TL::GetGameContext().m_camera.MoveTo(m_transform.m_position);

        // m_virtual_attack_button.Update();
    }

    void OnQuit() override {
        TL::Log("Quit");
    }

    private void attack() {
        m_weapon_script.Attack();
    }

    private void move(const TL::Vec2& in dir, TL::TimeType duration) {
        if (dir == TL::Vec2::ZERO) {
            if (m_anim_player !is null) {
                m_anim_player.Stop();
            }
        } else {
            if (!m_weapon_script.m_anim_player.IsPlaying()) {
                m_weapon_script.ChangeDir(dir.Normalize());
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

        if (m_cct !is null) {
            TL::Vec2 disp = dir * m_move_speed * duration;
            m_cct.MoveAndSlide(disp);
            m_transform.m_position = m_cct.GetPosition();
        }

        if ((!m_anim_player.IsPlaying() && dir != TL::Vec2::ZERO) ||
            old_direction != m_direction) {
            switch (m_direction) {
                case CharacterDirection::Up:
                    m_anim_player.ChangeAnimation(m_move_up_anim);
                    break;
                case CharacterDirection::Left:
                    m_anim_player.ChangeAnimation(m_move_left_anim);
                    break;
                case CharacterDirection::Right:
                    m_anim_player.ChangeAnimation(m_move_right_anim);
                    break;
                case CharacterDirection::Down:
                    m_anim_player.ChangeAnimation(m_move_down_anim);
                    break;
            }
            m_anim_player.Play();
        }

        if (dir == TL::Vec2::ZERO) {
            switch (m_direction) {
                case CharacterDirection::Up:
                    m_sprite.m_region.m_topleft = TL::Vec2(16, 0);
                    break;
                case CharacterDirection::Left:
                    m_sprite.m_region.m_topleft = TL::Vec2(32, 0);
                    break;
                case CharacterDirection::Right:
                    m_sprite.m_region.m_topleft = TL::Vec2(48, 0);
                    break;
                case CharacterDirection::Down:
                    m_sprite.m_region.m_topleft = TL::Vec2(0, 0);
                    break;
            }
        }

        m_sprite.m_z_order = TL::GetZOrderByYSorting(
            m_transform.m_position.y + m_sprite.m_size.y * 0.5f,
            TL::RenderLayer::TilemapArch);
    }

    private TL::AnimationPlayer@ m_anim_player;
    private TL::CharacterController@ m_cct;
    private float m_move_speed = 100;
    private CharacterDirection m_direction = CharacterDirection::Down;
    private TL::Transform@ m_transform;
    private TL::Handle<TL::Image> m_image_sheet;
    private TL::Sprite@ m_sprite;
    private TL::Handle<TL::Animation> m_move_left_anim;
    private TL::Handle<TL::Animation> m_move_right_anim;
    private TL::Handle<TL::Animation> m_move_up_anim;
    private TL::Handle<TL::Animation> m_move_down_anim;

    // weapon script instance
    private GenericForwardWeapon@ m_weapon_script;

    private TL::Entity m_weapon_entity;
}
