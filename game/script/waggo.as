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
        TL::Log("Initialize");
        @m_move_anim_player = TL::GetGameContext().m_animation_player_manager.Get(GetEntity());
        @m_cct = TL::GetGameContext().m_cct_manager.Get(GetEntity());
        @m_transform = TL::GetGameContext().m_transform_manager.Get(GetEntity());
    }

    void OnUpdate(TL::TimeType delta_time) {
        TL::Vec2 axises = TL::GetGameContext().m_input_manager.MakeAxises("MoveX", "MoveY")
                                    // TODO: use gamepad ID
                                    .Value(0);
        // auto& action = TL::GetGameContext().m_input_manager.GetAction("Attack");
        // if (action.IsPressed()) {
        //     Attack();
        // }

        Move(axises, delta_time);

        TL::GetGameContext().m_camera.MoveTo(m_transform.m_position);

        // m_virtual_attack_button.Update();

    }

    void OnQuit() {
        TL::Log("Quit");
    }

    private void Move(const TL::Vec2& in dir, TL::TimeType duration) {
        if (dir == TL::Vec2::ZERO) {
            if (m_move_anim_player !is null) {
                m_move_anim_player.Stop();
            }
        } else {
            // TODO: weapon animation
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

        // if ((!m_move_animation.IsPlaying() && dir != TL::Vec2::ZERO) ||
        //     old_direction != m_direction) {
        //     switch (m_direction) {
        //         case CharacterDirection::Up:
        //             m_move_anim_player.ChangeAnimation(m_move_up_animation);
        //             break;
        //         case CharacterDirection::Left:
        //             m_move_anim_player.ChangeAnimation(m_move_left_animation);
        //             break;
        //         case CharacterDirection::Right:
        //             m_move_anim_player.ChangeAnimation(m_move_right_animation);
        //             break;
        //         case CharacterDirection::Down:
        //             m_move_anim_player.ChangeAnimation(m_move_down_animation);
        //             break;
        //     }
        //     m_move_anim_player.Play();
        // }

        // if (dir == TL::Vec2::ZERO) {
        //     switch (m_direction) {
        //         case CharacterDirection::Up:
        //             m_sprite->m_region.m_topleft = {16, 0};
        //             break;
        //         case CharacterDirection::Left:
        //             m_sprite->m_region.m_topleft = {32, 0};
        //             break;
        //         case CharacterDirection::Right:
        //             m_sprite->m_region.m_topleft = {48, 0};
        //             break;
        //         case CharacterDirection::Down:
        //             m_sprite->m_region.m_topleft = {0, 0};
        //             break;
        //     }
        // }

        // m_sprite->m_z_order = GetZOrderByYSorting(
        //     m_transform->m_position.y + m_sprite->m_size.y * 0.5f,
        //     RenderLayer::TilemapArch);

    }

    private TL::AnimationPlayer@ m_move_anim_player;
    private TL::CharacterController@ m_cct;
    private float m_move_speed = 100;
    private CharacterDirection m_direction = CharacterDirection::Down;
    private TL::Transform@ m_transform;
}

/*
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

    if ((!m_move_animation->IsPlaying() && dir != TL::Vec2::ZERO) ||
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

    if (dir == TL::Vec2::ZERO) {
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

    m_sprite->m_z_order = GetZOrderByYSorting(
        m_transform->m_position.y + m_sprite->m_size.y * 0.5f,
        RenderLayer::TilemapArch);
}
*/
