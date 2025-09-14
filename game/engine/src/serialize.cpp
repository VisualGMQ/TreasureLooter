#include "engine/serialize.hpp"

#include "engine/context.hpp"
#include "engine/image.hpp"
#include "engine/physics.hpp"
#include "schema/serialize/anim.hpp"
#include "schema/serialize/anim_player.hpp"
#include "schema/serialize/flip.hpp"
#include <stdexcept>

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, long long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const long& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const int& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, int& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const short& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, short& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const char& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, char& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const unsigned long long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node,
                 unsigned long long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const unsigned long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const unsigned int& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned int& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const unsigned short& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned short& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const unsigned char& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned char& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Path& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(payload.string().c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Path& payload) {
    payload = node.value();
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc, Entity payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(
        std::to_string(static_cast<std::underlying_type_t<Entity>>(payload))
            .c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Entity& payload) {
    std::underlying_type_t<Entity> numeric;
    Deserialize(ctx, node, numeric);
    payload = static_cast<Entity>(numeric);
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(payload ? "true" : "false");
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, bool& payload) {
    std::string_view value = node.value();
    if (value == "true") {
        payload = true;
    } else if (value == "false") {
        payload = false;
    } else {
        LOGE("[Deserialize]: deserialize bool type failed, value: {}",
             node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const double& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const float& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, double& payload) {
    try {
        double value = std::stod(node.value());
        payload = value;
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stod exception: {}, {}", e.what(), node.value());
    }
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, float& payload) {
    try {
        float value = std::stof(node.value());
        payload = value;
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Region& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(ctx, doc, payload.m_topleft, "topleft"));
    node->append_node(Serialize(ctx, doc, payload.m_size, "size"));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Region& payload) {
    auto topleft_node = node.first_node("topleft");
    auto size_node = node.first_node("size");
    if (!topleft_node || !size_node) {
        LOGE("[Deserialize] parse Region failed! not topleft or size node");
        return;
    }
    Deserialize(ctx, *topleft_node, payload.m_topleft);
    Deserialize(ctx, *size_node, payload.m_size);
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Degrees& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload.Value()).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Degrees& payload) {
    try {
        payload = std::stof(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, value = {}", e.what(),
             node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Radians& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload.Value()).c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Radians& payload) {
    try {
        payload = std::stof(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, value = {}", e.what(),
             node.value());
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Transform& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(ctx, doc, payload.m_position, "position"));
    node->append_node(Serialize(ctx, doc, payload.m_scale, "scale"));
    node->append_node(Serialize(ctx, doc, payload.m_rotation, "rotation"));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Transform& payload) {
    auto position_node = node.first_node("position");
    auto scale_node = node.first_node("scale");
    auto rotation_node = node.first_node("rotation");

    if (!position_node || !scale_node || !rotation_node) {
        LOGE("[Deserialize] parse Pose failed! no position/scale/rotation "
             "field");
        return;
    }

    Deserialize(ctx, *position_node, payload.m_position);
    Deserialize(ctx, *scale_node, payload.m_scale);
    Deserialize(ctx, *rotation_node, payload.m_rotation);
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Handle<Image>& payload) {
    Path filename = node.value();
    auto& manager = CURRENT_CONTEXT.m_assets_manager->GetManager<Handle<T>>();
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Tilemap* payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    if (payload) {
        node->value(
            doc.allocate_string(payload->GetFilename().string().c_str()));
    }
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Tilemap*& payload) {
    Path filename = node.value();
    auto& manager = CURRENT_CONTEXT.m_assets_manager->GetManager<Tilemap>();
    auto handle = manager.Load(filename);
    if (!handle) {
        handle = manager.Load(filename);
    }

    if (!handle) {
        payload = handle.Get();
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Handle<Tilemap> payload,
                                const std::string& name) {
    return Serialize(ctx, doc, &*payload, name);
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Handle<Tilemap>& payload) {
    Path filename = node.value();
    auto& manager = CURRENT_CONTEXT.m_assets_manager->GetManager<Tilemap>();
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const std::string& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(payload.c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, std::string& payload) {
    Path filename = node.value();
    payload = node.value();
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const UUIDv4& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(payload.ToString().c_str()));
    return node;
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, UUIDv4& payload) {
    payload = UUIDv4::CreateFromString(node.value());
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const CollisionGroup& payload,
                                const std::string& name) {
    static_assert(
        std::is_unsigned_v<std::underlying_type_t<CollisionGroupType>>,
        "CollisionGroupType must be unsigned");

    std::vector<CollisionGroupType> types;
    for (int i = 0; i < sizeof(std::underlying_type_t<CollisionGroupType>);
         i++) {
        auto type = static_cast<CollisionGroupType>(1 << i);
        if (payload.Has(type)) {
            types.push_back(type);
        }
    }

    return Serialize(ctx, doc, types, doc.allocate_string(name.c_str()));
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, CollisionGroup& payload) {
    std::vector<CollisionGroupType> types;
    Deserialize(ctx, node, types);

    payload.Clear();
    for (auto& type : types) {
        payload.Add(type);
    }
}

// x-macros for shorten code
#define HANDLE_ANIM_SERIALIZE(binding) if (binding_point == binding)
#define HANDLE_LINEAR_TRACK_SERIALIZE()                                     \
    if (payload.GetType() == AnimationTrackType::Linear) {                  \
        auto& keyframes = static_cast<const AnimationTrack<                 \
            TARGET_TYPE, AnimationTrackType::Linear>&>(payload)             \
                              .GetKeyframes();                              \
        for (auto& keyframe : keyframes) {                                  \
            auto keyframe_node = Serialize(ctx, doc, keyframe, "keyframe"); \
            keyframes_node->append_node(keyframe_node);                     \
        }                                                                   \
    }
#define HANDLE_DISCRETE_TRACK_SERIALIZE()                                   \
    if (payload.GetType() == AnimationTrackType::Discrete) {                \
        auto& keyframes = static_cast<const AnimationTrack<                 \
            TARGET_TYPE, AnimationTrackType::Discrete>&>(payload)           \
                              .GetKeyframes();                              \
        for (auto& keyframe : keyframes) {                                  \
            auto keyframe_node = Serialize(ctx, doc, keyframe, "keyframe"); \
            keyframes_node->append_node(keyframe_node);                     \
        }                                                                   \
    }

rapidxml::xml_node<>* serializeAnimTrack(CommonContext& ctx, rapidxml::xml_document<>& doc,
                                         AnimationBindingPoint binding_point,
                                         const AnimationTrackBase& payload,
                                         const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(ctx, doc, binding_point, "binding_point"));
    auto keyframes_node =
        doc.allocate_node(rapidxml::node_type::node_element, "keyframes");
    node->append_node(Serialize(ctx, doc, payload.GetType(), "type"));
    node->append_node(keyframes_node);

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::TransformPosition) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Degrees
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::TransformRotation) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::TransformScale) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE ImageHandle
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteImage) {
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteFlip) {
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteRegionPosition) {
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteRegionSize) {
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteSize) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteAnchor) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::BindPoint) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

    return node;
}

#undef HANDLE_ANIM_SERIALIZE
#undef HANDLE_LINEAR_TRACK
#undef HANDLE_DISCRETE_TRACK

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const Animation& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    auto tracks_node =
        doc.allocate_node(rapidxml::node_type::node_element, "tracks");

    auto& tracks = payload.GetTracks();
    for (auto& track : tracks) {
        auto track_node =
            serializeAnimTrack(ctx, doc, track.first, *track.second, "track");
        tracks_node->append_node(track_node);
    }

    node->append_node(tracks_node);

    auto bind_point_tracks_node = doc.allocate_node(
        rapidxml::node_type::node_element, "bind_point_tracks");

    auto& bind_point_tracks = payload.GetBindPointTracks();
    for (auto& [name, track] : bind_point_tracks) {
        auto track_node = serializeAnimTrack(ctx,
            doc, AnimationBindingPoint::BindPoint, *track, "track");
        auto attribute_node =
            doc.allocate_attribute("name", doc.allocate_string(name.c_str()));
        track_node->append_attribute(attribute_node);
        bind_point_tracks_node->append_node(track_node);
    }

    node->append_node(bind_point_tracks_node);

    return node;
}

#define HANDLE_ANIM_DESERIALIZE(binding) if (binding_point == binding)
#define HANDLE_CREATE_TRACK()                          \
    std::vector<KeyFrame<TARGET_TYPE>> keyframes;      \
    while (keyframe_node) {                            \
        KeyFrame<TARGET_TYPE> keyframe;                \
        Deserialize(ctx, *keyframe_node, keyframe);         \
        keyframes.push_back(keyframe);                 \
        keyframe_node = keyframe_node->next_sibling(); \
    }
#define HANDLE_LINEAR_TRACK_DESERIALIZE()                               \
    if (type == AnimationTrackType::Linear) {                           \
        auto raw_track = std::make_unique<                              \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Linear>>(); \
        raw_track->AddKeyframes(std::move(keyframes));                  \
        track = std::move(raw_track);                                   \
    }
#define HANDLE_DISCRETE_TRACK_DESERIALIZE()                               \
    if (type == AnimationTrackType::Discrete) {                           \
        auto raw_track = std::make_unique<                                \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Discrete>>(); \
        raw_track->AddKeyframes(std::move(keyframes));                    \
        track = std::move(raw_track);                                     \
    }

std::tuple<AnimationBindingPoint, std::unique_ptr<AnimationTrackBase>>
deserializeTrack(CommonContext& ctx, rapidxml::xml_node<>& node) {
    AnimationBindingPoint binding_point = AnimationBindingPoint::Unknown;
    if (auto binding_point_node = node.first_node("binding_point")) {
        Deserialize(ctx, *binding_point_node, binding_point);
    }

    auto keyframes_node = node.first_node("keyframes");
    auto keyframe_node = keyframes_node->first_node("keyframe");
    AnimationTrackType type = AnimationTrackType::Discrete;
    auto type_node = node.first_node("type");
    Deserialize(ctx, *type_node, type);

    std::unique_ptr<AnimationTrackBase> track;

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::TransformPosition) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::TransformScale) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE float
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::TransformRotation) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE ImageHandle
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteImage) {
        HANDLE_CREATE_TRACK();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteFlip) {
        HANDLE_CREATE_TRACK();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteRegionPosition) {
        HANDLE_CREATE_TRACK();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteRegionSize) {
        HANDLE_CREATE_TRACK();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteSize) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteAnchor) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::BindPoint) {
        HANDLE_CREATE_TRACK();
        HANDLE_LINEAR_TRACK_DESERIALIZE();
        HANDLE_DISCRETE_TRACK_DESERIALIZE();
    }
#undef TARGET_TYPE

    return {binding_point, std::move(track)};
}

#undef HANDLE_ANIM_DESERIALIZE
#undef HANDLE_LINEAR_TRACK_DESERIALIZE
#undef HANDLE_DISCRETE_TRACK_DESERIALIZE

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Animation& payload) {
    auto tracks_node = node.first_node("tracks");
    if (tracks_node) {
        auto n = tracks_node->first_node();
        while (n) {
            auto [binding, track] = deserializeTrack(ctx, *n);
            payload.AddTrack(binding, std::move(track));
            n = n->next_sibling();
        }
    }

    auto bind_point_tracks_node = node.first_node("bind_point_tracks");
    if (bind_point_tracks_node) {
        auto n = bind_point_tracks_node->first_node();
        while (n) {
            auto name_attribute = n->first_attribute("name");
            auto [_, track] = deserializeTrack(ctx, *n);
            payload.AddBindPointTrack(
                name_attribute->value(),
                std::unique_ptr<IAnimationTrack<Vec2>>(
                    static_cast<IAnimationTrack<Vec2>*>(track.release())));
            n = n->next_sibling();
        }
    }
}

rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const AnimationPlayer& payload,
                                const std::string& name) {
    AnimationPlayerCreateInfo create_info;
    create_info.m_auto_play = payload.IsAutoPlayEnabled();
    create_info.m_animation = payload.GetAnimation();
    create_info.m_loop = payload.GetLoopCount();
    create_info.m_rate = payload.GetRate();

    return Serialize(ctx, doc, create_info, name);
}

void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, AnimationPlayer& payload) {
    AnimationPlayerCreateInfo create_info;
    Deserialize(ctx, node, create_info);

    payload.SetLoop(create_info.m_loop);
    payload.SetRate(create_info.m_rate);
    payload.EnableAutoPlay(create_info.m_auto_play);
    payload.ChangeAnimation(create_info.m_animation);
}