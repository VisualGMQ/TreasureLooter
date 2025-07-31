#include "animation.hpp"

#include "context.hpp"
#include "image.hpp"
#include "sprite.hpp"

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
    for (auto& [_, track] : m_tracks) {
        track->Rewind();
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
    for (auto& [_, track] : m_tracks) {
        track->Update(delta_time);
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

#define BEGIN_BINDING_POINT(binding)                                       \
    if (auto it = m_tracks.find(AnimationBindingPoint::TransformPosition); \
        it != m_tracks.end())

#define HANDLE_LINEAR_TRACK()                                               \
    if (it->second->GetType() == AnimationTrackType::Linear) {              \
        BINDING_TARGET =                                                    \
            static_cast<const AnimationTrack<decltype(BINDING_TARGET),      \
                                             AnimationTrackType::Linear>&>( \
                *it->second)                                                \
                .GetValue();                                                \
    }
#define HANDLE_DISCRETE_TRACK()                                               \
    if (it->second->GetType() == AnimationTrackType::Discrete) {              \
        BINDING_TARGET =                                                      \
            static_cast<const AnimationTrack<decltype(BINDING_TARGET),        \
                                             AnimationTrackType::Discrete>&>( \
                *it->second)                                                  \
                .GetValue();                                                  \
    }

void Animation::Sync(Entity entity) {
    auto& ctx = Context::GetInst();

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

#define BINDING_TARGET sprite->m_region
        BEGIN_BINDING_POINT(AnimationBindingPoint::SpriteRegion) {
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
    }
}

int Animation::GetLoopCount() const {
    return m_loop;
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
