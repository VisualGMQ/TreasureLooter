#include "engine/animation_player.hpp"

#include "engine/context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/profile.hpp"
#include "engine/sprite.hpp"

AnimationPlayer::AnimationPlayer(const AnimationPlayerCreateInfo& create_info) {
    EnableAutoPlay(create_info.m_auto_play);
    SetLoop(create_info.m_loop);
    SetRate(create_info.m_rate);
    ChangeAnimation(create_info.m_animation);
}

void AnimationPlayer::Play() {
    if (!m_animation) {
        return;
    }
    m_is_playing = true;
}

void AnimationPlayer::Pause() {
    if (!m_animation) {
        return;
    }
    m_is_playing = false;
}

void AnimationPlayer::Stop() {
    if (!m_animation) {
        return;
    }
    Rewind();
    Pause();
}

void AnimationPlayer::Rewind() {
    if (!m_animation) {
        return;
    }
    for (auto& [_, track] : m_track_players) {
        track->Rewind();
    }
    m_cur_time = 0.0f;
}

void AnimationPlayer::SetLoop(int loop) {
    m_loop = loop;
}

bool AnimationPlayer::IsPlaying() const {
    return m_is_playing;
}

int AnimationPlayer::GetLoopCount() const {
    return m_loop;
}

TimeType AnimationPlayer::GetCurTime() const {
    return m_cur_time;
}

TimeType AnimationPlayer::GetMaxTime() const {
    if (!m_animation) {
        return 0;
    }
    TimeType max_time = 0;
    for (auto& [_, track] : m_animation->GetTracks()) {
        max_time = std::max(max_time, track->GetFinishTime());
    }
    for (auto& [_, track] : m_animation->GetBindPointTracks()) {
        max_time = std::max(max_time, track->GetFinishTime());
    }
    return max_time;
}

#define HANDLE_TRACK_BINDING_POINT(binding) if (binding_point == binding)
#define HANDLE_LINEAR_TRACK_CREATION()                                         \
    if (track->GetType() == AnimationTrackType::Linear) {                      \
        auto& raw_track = static_cast<                                         \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Linear>&>(*track); \
        m_track_players[binding_point] = std::make_unique<                     \
            AnimationTrackPlayer<TARGET_TYPE, AnimationTrackType::Linear>>(    \
            raw_track);                                                        \
    }
#define HANDLE_DISCRETE_TRACK_CREATION()                                      \
    if (track->GetType() == AnimationTrackType::Discrete) {                   \
        auto& raw_track = static_cast<                                        \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Discrete>&>(      \
            *track);                                                          \
        m_track_players[binding_point] = std::make_unique<                    \
            AnimationTrackPlayer<TARGET_TYPE, AnimationTrackType::Discrete>>( \
            raw_track);                                                       \
    }

void AnimationPlayer::ChangeAnimation(AnimationHandle animation) {
    m_animation = animation;
    m_track_players.clear();

    Stop();

    if (!m_animation) {
        return;
    }

    if (m_auto_play) {
        Play();
    }

    auto& tracks = m_animation->GetTracks();
    for (auto& [binding_point, track] : tracks) {
#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::TransformPosition) {
            HANDLE_LINEAR_TRACK_CREATION();
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::TransformScale) {
            HANDLE_LINEAR_TRACK_CREATION();
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Degrees
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::TransformRotation) {
            HANDLE_LINEAR_TRACK_CREATION();
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteFlip) {
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE ImageHandle
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteImage) {
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(
            AnimationBindingPoint::SpriteRegionPosition) {
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteRegionSize) {
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteSize) {
            HANDLE_LINEAR_TRACK_CREATION()
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteAnchor) {
            HANDLE_LINEAR_TRACK_CREATION()
            HANDLE_DISCRETE_TRACK_CREATION();
        }
#undef TARGET_TYPE
    }

    auto& bind_point_tracks = m_animation->GetBindPointTracks();
    for (auto& [name, track] : bind_point_tracks) {
        if (track->GetType() == AnimationTrackType::Linear) {
            auto& raw_track =
                static_cast<AnimationTrack<Vec2, AnimationTrackType::Linear>&>(
                    *track);
            m_bind_point_track_players.emplace(
                name,
                std::make_unique<
                    AnimationTrackPlayer<Vec2, AnimationTrackType::Linear>>(
                    raw_track));
        }
        if (track->GetType() == AnimationTrackType::Discrete) {
            auto& raw_track = static_cast<
                AnimationTrack<Vec2, AnimationTrackType::Discrete>&>(*track);
            m_bind_point_track_players.emplace(
                name,
                std::make_unique<
                    AnimationTrackPlayer<Vec2, AnimationTrackType::Discrete>>(
                    raw_track));
        }
    }
}

#undef HANDLE_LINEAR_TRACK_CREATION
#undef HANDLE_DISCRETE_TRACK_CREATION
#undef HANDLE_TRACK_BINDING_POINT

void AnimationPlayer::ChangeAnimation(const Path& filename) {
    auto animation =
        CURRENT_CONTEXT.m_assets_manager->GetManager<Animation>().Find(filename);
    ChangeAnimation(animation);
}

void AnimationPlayer::ChangeAnimation(UUID uuid) {
    auto animation =
        CURRENT_CONTEXT.m_assets_manager->GetManager<Animation>().Find(uuid);
    ChangeAnimation(animation);
}

void AnimationPlayer::ClearAnimation() {
    m_animation = nullptr;
}

bool AnimationPlayer::HasAnimation() const {
    return m_animation;
}

void AnimationPlayer::Update(TimeType delta_time) {
    float elapsed_time = delta_time * m_rate;

    if (!m_is_playing || !m_animation || m_track_players.empty()) {
        return;
    }

    m_cur_time += elapsed_time;
    for (auto& [_, track] : m_track_players) {
        track->Update(elapsed_time);
    }
    for (auto& [_, track] : m_bind_point_track_players) {
        track->Update(elapsed_time);
    }

    if (m_cur_time >= GetMaxTime()) {
        if (m_loop > 0 || m_loop == InfLoop) {
            TimeType backup_time = m_cur_time;
            Rewind();
            m_cur_time = backup_time - GetMaxTime();

            for (auto& [_, track] : m_track_players) {
                track->Update(m_cur_time);
            }
            for (auto& [_, track] : m_bind_point_track_players) {
                track->Update(elapsed_time);
            }

            if (m_loop != InfLoop) {
                m_loop--;
            }
        } else {
            Pause();
            m_cur_time = GetMaxTime();
        }
    }
}

#define BEGIN_BINDING_POINT(binding) \
    if (auto it = m_track_players.find(binding); it != m_track_players.end())

#define HANDLE_LINEAR_TRACK()                                        \
    if (it->second->GetType() == AnimationTrackType::Linear) {       \
        auto& raw_track = static_cast<const AnimationTrackPlayer<    \
            decltype(BINDING_TARGET), AnimationTrackType::Linear>&>( \
            *it->second);                                            \
        if (raw_track.NeedSync()) {                                  \
            BINDING_TARGET = raw_track.GetValue();                   \
        }                                                            \
    }
#define HANDLE_DISCRETE_TRACK()                                        \
    if (it->second->GetType() == AnimationTrackType::Discrete) {       \
        auto& raw_track = static_cast<const AnimationTrackPlayer<      \
            decltype(BINDING_TARGET), AnimationTrackType::Discrete>&>( \
            *it->second);                                              \
        if (raw_track.NeedSync()) {                                    \
            BINDING_TARGET = raw_track.GetValue();                     \
        }                                                              \
    }

void AnimationPlayer::Sync(Entity entity) {
    auto& ctx = CURRENT_CONTEXT;

    if (auto transform = ctx.m_transform_manager->Get(entity)) {
#define BINDING_TARGET transform->m_position
        BEGIN_BINDING_POINT(AnimationBindingPoint::TransformPosition) {
            HANDLE_LINEAR_TRACK();
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET transform->m_scale
        BEGIN_BINDING_POINT(AnimationBindingPoint::TransformScale) {
            HANDLE_LINEAR_TRACK();
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET transform->m_rotation
        BEGIN_BINDING_POINT(AnimationBindingPoint::TransformRotation) {
            HANDLE_LINEAR_TRACK();
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET
    }

    if (auto sprite = ctx.m_sprite_manager->Get(entity)) {
#define BINDING_TARGET sprite->m_image
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteImage) {
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET sprite->m_region.m_topleft
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteRegionPosition) {
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET sprite->m_region.m_size
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteRegionSize) {
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET sprite->m_size
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteSize) {
            HANDLE_LINEAR_TRACK();
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET sprite->m_flip
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteFlip) {
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET

#define BINDING_TARGET sprite->m_anchor
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteAnchor) {
            HANDLE_DISCRETE_TRACK();
            HANDLE_LINEAR_TRACK();
        }
#undef BINDING_TARGET
    }

    if (auto bind_points = ctx.m_bind_point_component_manager->Get(entity)) {
        for (auto& [name, bind_point] : bind_points->m_bind_points) {
            if (auto it = m_bind_point_track_players.find(name);
                it != m_bind_point_track_players.end()) {
#define BINDING_TARGET bind_point.m_position
                HANDLE_DISCRETE_TRACK();
                HANDLE_LINEAR_TRACK();
#undef BINDING_TARGET
            }
        }
    }
}

void AnimationPlayer::SetRate(float rate) {
    m_rate = std::max(0.0f, rate);
}

float AnimationPlayer::GetRate() const {
    return m_rate;
}

AnimationHandle AnimationPlayer::GetAnimation() const {
    return m_animation;
}

void AnimationPlayer::EnableAutoPlay(bool enable) {
    m_auto_play = enable;
}

bool AnimationPlayer::IsAutoPlayEnabled() const {
    return m_auto_play;
}

void AnimationPlayerManager::Update(TimeType delta_time) {
    PROFILE_ANIMATION_SECTION(__FUNCTION__);
    
    for (auto& [entity, anim] : m_components) {
        if (!anim.m_enable) {
            continue;
        }
        anim.m_component->Update(delta_time);
        anim.m_component->Sync(entity);
    }
}
