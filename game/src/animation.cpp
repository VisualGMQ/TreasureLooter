#include "animation.hpp"

#include "context.hpp"

void Animation::Play() {
    m_is_playing = true;
}

void Animation::Pause() {
    m_is_playing = false;
}

void Animation::Stop() {
    Rewind();
    Pause();
}

void Animation::Rewind() {
    for (auto& track : m_tracks) {
        if (track) {
            track->Rewind();
        }
    }
}

void Animation::SetLoop(int loop) {
    m_loop = loop;
}

void Animation::Update(TimeType delta_time) {
    if (!m_is_playing) {
        return;
    }

    m_cur_time += delta_time;
    for (auto& track : m_tracks) {
        if (track) {
            track->Update(delta_time);
        }
    }

    if (m_cur_time >= m_max_time) {
        if (m_loop > 0 || m_loop == InfLoop) {
            Rewind();
            m_cur_time = m_cur_time - m_max_time;

            if (m_loop != InfLoop) {
                m_loop--;
            }
        } else {
            Pause();
            m_cur_time = m_max_time;
        }
    }
}

void Animation::Sync(Entity entity) {
    auto& ctx = Context::GetInst();

    if (m_tracks[static_cast<size_t>(
            AnimationBindingPoint::TransformPositionX)] ||
        m_tracks[static_cast<size_t>(
            AnimationBindingPoint::TransformPositionY)]) {
        if (auto transform = ctx.m_transform_manager->Get(entity); transform) {
            if (m_tracks[static_cast<size_t>(
                    AnimationBindingPoint::TransformPositionX)]) {
                transform->m_position.x =
                    static_cast<
                        AnimationTrack<float, AnimationTrackType::Linear>*>(
                        m_tracks[static_cast<size_t>(
                                     AnimationBindingPoint::TransformPositionX)]
                            .get())
                        ->GetValue();
            }
            if (m_tracks[static_cast<size_t>(
                    AnimationBindingPoint::TransformPositionY)]) {
                transform->m_position.y =
                    static_cast<
                        AnimationTrack<float, AnimationTrackType::Linear>*>(
                        m_tracks[static_cast<size_t>(
                                     AnimationBindingPoint::TransformPositionY)]
                            .get())
                        ->GetValue();
            }
        }
    }
}

void AnimationComponentManager::Update(TimeType delta_time) {
    for (auto& [entity, anim] : m_components) {
        anim->Update(delta_time);
        anim->Sync(entity);
    }
}

AnimationHandle AnimationManager::Create() {
    return store(nullptr, UUID::CreateV4(), std::make_unique<Animation>());
}