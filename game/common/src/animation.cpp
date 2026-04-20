#include "common/animation.hpp"

#include "common/context.hpp"
#include "common/image.hpp"
#include "common/serialize.hpp"
#include "common/storage.hpp"
#include "rapidxml_print.hpp"

#include <sstream>

AnimationHandle AnimationManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }

    auto result = LoadAsset<Animation>(filename);
    if (!result) {
        LOGE("load animation {} failed", filename);
        return {};
    }

    return store(&filename, result.m_uuid, std::move(result.m_payload));
}

void Animation::AddTrack(AnimationBindingPoint binding,
                         std::unique_ptr<AnimationTrackBase>&& track) {
    if (track && !track->IsEmpty()) {
        m_tracks[binding] = std::move(track);
    }
}

void Animation::AddBindPointTrack(
    const std::string& name, std::unique_ptr<IAnimationTrack<Vec2>>&& track) {
    m_bind_point_tracks.emplace(name, std::move(track));
}

void Animation::AddTracks(const SpriteRowColumnAnimationInfo& info) {
    if (info.m_begin == info.m_end) {
        return;
    }

    auto region_position_track =
        std::make_unique<AnimationTrack<Vec2, AnimationTrackType::Discrete>>();
    for (auto i = info.m_begin; i != info.m_end; ++i) {
        Vec2 topleft;
        if (info.m_is_row) {
            topleft.x = i * info.m_region_size.w;
            topleft.y = info.m_other_dim * info.m_region_size.h;
        } else {
            topleft.x = info.m_other_dim * info.m_region_size.w;
            topleft.y = i * info.m_region_size.h;
        }

        TimeType time = info.m_duration * (i - info.m_begin);

        region_position_track->AddKeyframe(KeyFrame<Vec2>{topleft, time});
    }

    auto last_keyframe = region_position_track->GetKeyframes().back();
    last_keyframe.m_time += info.m_duration;
    region_position_track->AddKeyframe(last_keyframe);
    m_tracks[AnimationBindingPoint::SpriteRegionPosition] =
        std::move(region_position_track);

    auto sprite_track = std::make_unique<
        AnimationTrack<ImageHandle, AnimationTrackType::Discrete>>();
    sprite_track->AddKeyframe({info.m_image, 0});
    m_tracks[AnimationBindingPoint::SpriteImage] = std::move(sprite_track);

    auto region_size_track =
        std::make_unique<AnimationTrack<Vec2, AnimationTrackType::Discrete>>();
    region_size_track->AddKeyframe({info.m_region_size, 0});
    m_tracks[AnimationBindingPoint::SpriteRegionSize] =
        std::move(region_size_track);
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

    auto result = LoadAsset<Animation>(*node);
    TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(result, LOGE, "load asset {} failed",
                                        filename);
    return result;
}

template <>
AssetLoadResult<Animation> LoadAsset<Animation>(
    const rapidxml::xml_node<>& node) {
    auto uuid_node = node.first_node("uuid");
    TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(
        uuid_node, LOGE, "parse asset {} failed, no node", "uuid");

    auto value_node = node.first_node("payload");
    TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(
        value_node, LOGE, "parse asset {} failed, no node", "payload");

    AssetLoadResult<Animation> result;
    Deserialize(COMMON_CONTEXT, *uuid_node, result.m_uuid);
    result.m_payload = std::make_unique<Animation>();
    Deserialize(COMMON_CONTEXT, *value_node, *result.m_payload);
    return result;
}

void SaveAsset(const UUID& uuid, const Animation& payload,
               const Path& filename) {
    rapidxml::xml_document<> doc;

    auto value_node = Serialize(COMMON_CONTEXT, doc, payload, "payload");
    if (!value_node) {
        LOGE("save asset {} failed", filename);
    }
    auto uuid_node = Serialize(COMMON_CONTEXT, doc, uuid, "uuid");
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
