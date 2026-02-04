#include "behavior.as"

shared class GenericForwardWeapon: TL::Behavior {
    GenericForwardWeapon(TL::Entity entity) {
        super(entity);
    }

    void OnInit() {
        @m_anim_player = GetAnimationPlayerComponent();
        @m_transform = GetTransformComponent();
    }

    void OnUpdate(TL::TimeType delta_time) {}

    void OnQuit() {}

    void ChangeDir(const TL::Vec2& in dir) {
        m_dir = dir;
        m_transform.m_rotation = TL::GetAngle(m_dir, TL::Vec2::X_UNIT);
    }

    void Attack() {
        if ((m_anim_player !is null) && !m_anim_player.IsPlaying()) {
            m_anim_player.Stop();
            m_anim_player.Play();
        }
        if (m_transform !is null) {
            TL::Radians angle = TL::GetAngle(m_dir, TL::Vec2::X_UNIT);
            m_transform.m_rotation = angle;
        }
    }

    TL::Transform@ m_transform;
    TL::AnimationPlayer@ m_anim_player;
    private TL::Vec2 m_dir = TL::Vec2::X_UNIT;
}
