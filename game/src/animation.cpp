#include "animation.hpp"

#include "context.hpp"
#include "image.hpp"
#include "rapidxml_print.hpp"
#include "serialize.hpp"
#include "sprite.hpp"
#include "storage.hpp"

#include <sstream>

Animation::Animation(const Path& filename) : m_filename{filename} {}

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
    m_cur_time = 0.0f;
}

void Animation::SetLoop(int loop) {
    m_loop = loop;
}

bool Animation::IsPlaying() const {
    return m_is_playing;
}

void Animation::Update(TimeType delta_time) {
    if (!m_is_playing) {
        return;
    }

    m_cur_time += delta_time;
    for (auto& [_, track] : m_tracks) {
        track->Update(delta_time);
    }

    if (m_cur_time >= GetMaxTime()) {
        if (m_loop > 0 || m_loop == InfLoop) {
            TimeType backup_time = m_cur_time;
            Rewind();
            m_cur_time = backup_time - GetMaxTime();

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
    if (auto it = m_tracks.find(binding); it != m_tracks.end())

#define HANDLE_LINEAR_TRACK()                                               \
    if (it->second->GetType() == AnimationTrackType::Linear) {              \
        auto& raw_track =                                                   \
            static_cast<const AnimationTrack<decltype(BINDING_TARGET),      \
                                             AnimationTrackType::Linear>&>( \
                *it->second);                                               \
        if (raw_track.NeedSync()) {                                         \
            BINDING_TARGET = raw_track.GetValue();                          \
        }                                                                   \
    }
#define HANDLE_DISCRETE_TRACK()                                               \
    if (it->second->GetType() == AnimationTrackType::Discrete) {              \
        auto& raw_track =                                                     \
            static_cast<const AnimationTrack<decltype(BINDING_TARGET),        \
                                             AnimationTrackType::Discrete>&>( \
                *it->second);                                                 \
        if (raw_track.NeedSync()) {                                           \
            BINDING_TARGET = raw_track.GetValue();                            \
        }                                                                     \
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

TimeType Animation::GetCurTime() const {
    return m_cur_time;
}

TimeType Animation::GetMaxTime() const {
    TimeType max_time = 0;
    for (auto& [_, track] : m_tracks) {
        max_time = std::max(max_time, track->GetFinishTime());
    }
    return max_time;
}

const Path& Animation::Filename() const {
    return m_filename;
}

void AnimationComponentManager::Update(TimeType delta_time) {
    for (auto& [entity, anim] : m_components) {
        anim->Get()->Update(delta_time);
        anim->Get()->Sync(entity);
    }
}

AnimationHandle AnimationManager::Load(const Path& filename) {
    auto result = LoadAsset<Animation>(filename);
    if (!result.m_uuid) {
        LOGE("load animation {} failed! uuid is null", filename);
        return {};
    }

    return store(&filename, result.m_uuid,
                 std::make_unique<Animation>(std::move(result.m_payload)));
}

AnimationHandle AnimationManager::Create() {
    return store(nullptr, UUID::CreateV4(), std::make_unique<Animation>());
}

template <>
AssetLoadResult<Animation> LoadAsset<Animation>(const Path& filename) {
    auto file = IOStream::CreateFromFile(filename, IOMode::Read, true);
    auto content = file->Read();
    content.push_back('\0');
    rapidxml::xml_document<> doc;
    try {
        doc.parse<rapidxml::parse_default>(content.data());
    } catch (std::exception& e) {
        LOGE("parse asset {} failed: {}", filename, e.what());
    }
    auto node = doc.first_node("Animation");
    if (!node) {
        LOGE("parse asset {} failed, no node {}", filename, "Animation");
    }

    auto uuid_node = node->first_node("uuid");
    if (!uuid_node) {
        LOGE("parse asset {} failed, no node {}", filename, "uuid");
    }

    auto value_node = node->first_node("value");
    if (!value_node) {
        LOGE("parse asset {} failed, no node {}", filename, "value");
    }

    AssetLoadResult<Animation> result;
    Deserialize(*uuid_node, result.m_uuid);
    Deserialize(*value_node, result.m_payload);
    return result;
}

void SaveAsset(const UUID& uuid, const Animation& payload,
               const Path& filename) {
    rapidxml::xml_document<> doc;

    auto value_node = Serialize(doc, payload, "value");
    if (!value_node) {
        LOGE("save asset {} failed", filename);
    }
    auto uuid_node = Serialize(doc, uuid, "uuid");
    if (!uuid_node) {
        LOGE("save asset {} failed", filename);
    }

    auto node =
        doc.allocate_node(rapidxml::node_type::node_element, "Animation");
    node->append_node(uuid_node);
    node->append_node(value_node);

    doc.append_node(node);
    std::stringstream ss;
    ss << doc;
    std::string content = ss.str();
    auto io = IOStream::CreateFromFile(filename, IOMode::Write, false, true);
    io->Write(content.data(), content.size());
}
