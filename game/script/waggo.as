enum CharacterDirection {
    Left,
    Right,
    Down,
    Up,
}

class MyClass : TL::Behavior {
    MyClass(TL::Entity entity) {
        super(entity);
    }

    void OnInit() {
        TL::Entity entity = GetEntity();
        TL::GameContext@ ctx = TL::GetGameContext();

        TL::GameplayConfig@ config = ctx.m_gameplay_config_manager.Get(entity);

        @m_anim_player = ctx.m_animation_player_manager.Get(entity);
        @m_cct = ctx.m_cct_manager.Get(entity);
        @m_transform = ctx.m_transform_manager.Get(entity);
        @m_sprite = ctx.m_sprite_manager.Get(entity);
        m_image_sheet = config.m_sprite_sheet;
        m_move_speed = config.m_speed;
        m_move_left_anim = config.m_move_left_animation;
        m_move_right_anim = config.m_move_right_animation;
        m_move_up_anim = config.m_move_up_animation;
        m_move_down_anim = config.m_move_down_animation;

        TL::Relationship@ relationship = ctx.m_relationship_manager.Get(entity);
        if (!relationship.m_children.isEmpty()) {
            if (config.m_weapon_entity.Has()) {
                uint32 idx = config.m_weapon_entity.Value();
                if (idx < relationship.m_children.length()) {
                    m_weapon_entity = relationship.m_children[idx];
                }
            }
        }
    }

    void OnUpdate(TL::TimeType delta_time) {
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

    void OnQuit() {
        TL::Log("Quit");
    }

    private void attack() {
        if (m_weapon_entity.IsNull()) {
            return;
        }

        TL::AnimationPlayer@ weapon_attack_animator =
            TL::GetGameContext().m_animation_player_manager.Get(m_weapon_entity);
        TL::Transform@ weapon_transform =
            TL::GetGameContext().m_transform_manager.Get(m_weapon_entity);
        if ((weapon_attack_animator !is null) && !weapon_attack_animator.IsPlaying()) {
            weapon_attack_animator.Stop();
            weapon_attack_animator.Play();
        }
        if (weapon_transform !is null) {
            TL::Radians angle = TL::GetAngle(m_weapon_dir, TL::Vec2::X_UNIT);
            weapon_transform.m_rotation = angle;
        }
    }

    private void move(const TL::Vec2& in dir, TL::TimeType duration) {
        if (dir == TL::Vec2::ZERO) {
            if (m_anim_player !is null) {
                m_anim_player.Stop();
            }
        } else {
            TL::AnimationPlayer@ weapon_anim = TL::GetGameContext().m_animation_player_manager.Get(m_weapon_entity);
            if ((weapon_anim !is null) && !weapon_anim.IsPlaying()) {
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

    private TL::Vec2 m_weapon_dir = TL::Vec2::X_UNIT;
    private TL::Entity m_weapon_entity;
}