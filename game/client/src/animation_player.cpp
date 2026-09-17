#include "client/animation_player.hpp"

#include "client/context.hpp"
#include "client/sprite.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/profile.hpp"

std::underlying_type_t<AnimationPlayerID> AnimationPlayer::next_id = 1;

AnimationPlayer::AnimationPlayer() : m_id{static_cast<AnimationPlayerID>(next_id++)} {}

AnimationPlayer::AnimationPlayer(const AnimationPlayerDefinition& create_info)
    : m_id{static_cast<AnimationPlayerID>(next_id++)} {
    EnableAutoPlay(create_info.m_auto_play);
    SetLoop(create_info.m_loop);
    SetRate(create_info.m_rate);
    ChangeAnimation(create_info.m_animation);
}

AnimationPlayerID AnimationPlayer::GetID() const {
    return m_id;
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
    for (auto& [_, track] : m_bind_point_track_players) {
        track->Rewind();
    }
    m_cur_time = 0.0f;
}

void AnimationPlayer::SetCurTime(TimeType time) {
    if (!m_animation) {
        return;
    }
    Rewind();
    Update(time);
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

#define TARGET_TYPE Vec2
        HANDLE_TRACK_BINDING_POINT(
            AnimationBindingPoint::TransformPositionOffset) {
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

#define TARGET_TYPE Color
        HANDLE_TRACK_BINDING_POINT(AnimationBindingPoint::SpriteColor) {
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
        CLIENT_CONTEXT.m_assets_manager->GetManager<Animation>().Find(filename);
    ChangeAnimation(animation);
}

void AnimationPlayer::ChangeAnimation(UUIDv4 uuid) {
    auto animation =
        CLIENT_CONTEXT.m_assets_manager->GetManager<Animation>().Find(uuid);
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

    if (!m_is_playing || !m_animation ||
        (m_track_players.empty() && m_bind_point_track_players.empty())) {
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
            COMMON_CONTEXT.m_event_system->EnqueueEvent<AnimationEndEvent>(
                AnimationEndEvent{m_id, ToLogicEntity(m_entity), m_animation});
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

// Same as above but added on top of the current value, for binding points
// defined as an offset (the value mirrored from the logic transform is
// refreshed every frame, so this doesn't accumulate).
#define HANDLE_LINEAR_TRACK_ADD()                                      \
    if (it->second->GetType() == AnimationTrackType::Linear) {         \
        auto& raw_track = static_cast<const AnimationTrackPlayer<      \
            decltype(BINDING_TARGET), AnimationTrackType::Linear>&>(   \
            *it->second);                                              \
        if (raw_track.NeedSync()) {                                    \
            BINDING_TARGET = BINDING_TARGET + raw_track.GetValue();    \
        }                                                              \
    }
#define HANDLE_DISCRETE_TRACK_ADD()                                    \
    if (it->second->GetType() == AnimationTrackType::Discrete) {       \
        auto& raw_track = static_cast<const AnimationTrackPlayer<      \
            decltype(BINDING_TARGET), AnimationTrackType::Discrete>&>( \
            *it->second);                                              \
        if (raw_track.NeedSync()) {                                    \
            BINDING_TARGET = BINDING_TARGET + raw_track.GetValue();    \
        }                                                              \
    }

void AnimationPlayer::Sync(PresentEntity entity) {
    TL_RETURN_IF_FALSE(m_animation);
    m_entity = entity;
    auto& ctx = CLIENT_CONTEXT;

    if (auto transform = ctx.m_present_transform_manager->Get(entity)) {
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

#define BINDING_TARGET transform->m_position
        BEGIN_BINDING_POINT(
            AnimationBindingPoint::TransformPositionOffset) {
            HANDLE_LINEAR_TRACK_ADD();
            HANDLE_DISCRETE_TRACK_ADD();
        }
#undef BINDING_TARGET

        // The render systems read the cached global matrix (const
        // Transform::GetGlobalMat()), so refresh it after the transform tracks
        // wrote into this render-only transform.
        transform->UpdateMat();
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

#define BINDING_TARGET sprite->m_color
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteColor) {
            HANDLE_LINEAR_TRACK();
            HANDLE_DISCRETE_TRACK();
        }
#undef BINDING_TARGET
    }

    if (auto bind_points =
            ctx.m_bind_point_component_manager->Get(ToLogicEntity(entity))) {
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
    PROFILE_SECTION();

    for (auto& [entity, components] : m_components) {
        for (auto& component : components) {
            if (!component.m_enable) {
                continue;
            }
            component.m_component->Update(delta_time);
            component.m_component->Sync(entity);
        }
    }
}
