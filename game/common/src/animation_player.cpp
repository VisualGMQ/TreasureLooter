#include "common/animation_player.hpp"

#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/context.hpp"
#include "common/profile.hpp"

std::underlying_type_t<AnimationPlayerID> AnimationPlayer::g_next_id = 1;
std::unordered_map<AnimationBindingPoint, AnimationPlayer::TrackInfo>
    AnimationPlayer::g_track_infos{};

AnimationPlayer::AnimationPlayer()
    : m_id{static_cast<AnimationPlayerID>(g_next_id++)} {}

AnimationPlayer::AnimationPlayer(const AnimationPlayerDefinition& create_info)
    : m_id{static_cast<AnimationPlayerID>(g_next_id++)} {
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
        if (auto it = g_track_infos.find(binding_point);
            it != g_track_infos.end()) {
            if (track->GetType() == AnimationTrackType::Linear) {
                it->second.m_linear_create_function(*this, *track);
            } else {
                it->second.m_discrete_create_function(*this, *track);
            }
        }
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

void AnimationPlayer::ChangeAnimation(const Path& filename) {
    auto animation =
        COMMON_CONTEXT.m_assets_manager->GetManager<Animation>().Find(filename);
    ChangeAnimation(animation);
}

void AnimationPlayer::ChangeAnimation(UUIDv4 uuid) {
    auto animation =
        COMMON_CONTEXT.m_assets_manager->GetManager<Animation>().Find(uuid);
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
                AnimationEndEvent{m_id, m_entity, m_animation});
        }
    }
}

void AnimationPlayer::Sync(Entity entity) {
    TL_RETURN_IF_FALSE(m_animation);
    m_entity = entity;

    for (auto& [bind_point, track] : m_track_players) {
        if (auto it = g_track_infos.find(bind_point);
            it != g_track_infos.end()) {
            if (track->GetType() == AnimationTrackType::Linear) {
                it->second.m_linear_sync_function(&entity, *track);
            } else {
                it->second.m_discrete_sync_function(&entity, *track);
            }
        }
    }

    if (auto bind_points =
            COMMON_CONTEXT.m_bind_point_component_manager->Get(entity)) {
        for (auto& [name, bind_point] : bind_points->m_bind_points) {
            if (auto it = m_bind_point_track_players.find(name);
                it != m_bind_point_track_players.end()) {
                auto bind_point_it = bind_points->m_bind_points.find(name);
                TL_CONTINUE_IF_FALSE(bind_point_it !=
                                     bind_points->m_bind_points.end());

                auto& track_player_base = *it->second;
                if (track_player_base.GetType() == AnimationTrackType::Linear) {
                    auto& track_player = static_cast<AnimationTrackPlayer<
                        Vec2, AnimationTrackType::Linear>&>(track_player_base);
                    TL_CONTINUE_IF_FALSE(track_player.NeedSync());

                    bind_point_it->second.m_position = track_player.GetValue();
                } else {
                    auto& track_player = static_cast<AnimationTrackPlayer<
                        Vec2, AnimationTrackType::Discrete>&>(
                        track_player_base);
                    TL_CONTINUE_IF_FALSE(track_player.NeedSync());

                    bind_point_it->second.m_position = track_player.GetValue();
                }
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

MultiAnimationPlayer::MultiAnimationPlayer(
    const MultiAnimationPlayerDefinition& definition) {
    for (auto& def : definition.m_animations) {
        AddAnimation(AnimationPlayer{def});
    }
}

const AnimationPlayer& MultiAnimationPlayer::GetAnimation(size_t index) const {
    return m_players[index];
}

AnimationPlayer& MultiAnimationPlayer::GetAnimation(size_t index) {
    return const_cast<AnimationPlayer&>(
        std::as_const(*this).GetAnimation(index));
}

void MultiAnimationPlayer::AddAnimation(AnimationPlayer&& o) {
    m_players.emplace_back(std::move(o));
}

AnimationPlayer& MultiAnimationPlayer::AddAnimation(
    const AnimationPlayerDefinition& def) {
    AnimationPlayer& player = m_players.emplace_back(def);
    return player;
}

AnimationPlayer& MultiAnimationPlayer::AddAnimation(AnimationHandle handle) {
    AnimationPlayer& player = m_players.emplace_back();
    player.ChangeAnimation(handle);
    return player;
}

void MultiAnimationPlayer::RemoveAnimation(const AnimationPlayer& player) {
    m_players.erase(
        std::remove_if(m_players.begin(), m_players.end(),
                       [&](AnimationPlayer& o) { return &o == &player; }),
        m_players.end());
}

std::vector<AnimationPlayer>& MultiAnimationPlayer::GetAnimations() {
    return const_cast<std::vector<AnimationPlayer>&>(
        std::as_const(*this).GetAnimations());
}

const std::vector<AnimationPlayer>& MultiAnimationPlayer::GetAnimations()
    const {
    return m_players;
}

void MultiAnimationPlayer::Update(TimeType elapse_time) {
    for (auto& player : m_players) {
        player.Update(elapse_time);
    }
}

void MultiAnimationPlayer::Sync(Entity entity) {
    for (auto& player : m_players) {
        player.Sync(entity);
    }
}

void MultiAnimationPlayer::PlayAll() {
    for (auto& player : m_players) {
        player.Play();
    }
}

void MultiAnimationPlayer::PauseAll() {
    for (auto& player : m_players) {
        player.Pause();
    }
}

void MultiAnimationPlayer::StopAll() {
    for (auto& player : m_players) {
        player.Stop();
    }
}

void MultiAnimationPlayer::RewindAll() {
    for (auto& player : m_players) {
        player.Rewind();
    }
}

void MultiAnimationPlayerManager::Update(TimeType delta_time) {
    PROFILE_SECTION();

    for (auto& [entity, anim] : m_components) {
        if (!anim.m_enable) {
            continue;
        }
        anim.m_component->Update(delta_time);
        anim.m_component->Sync(entity);
    }
}
