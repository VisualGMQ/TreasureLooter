#include "serialize.hpp"

#include "context.hpp"
#include "image.hpp"
#include "schema/serialize/anim.hpp"
#include "schema/serialize/flip.hpp"
#include <stdexcept>

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, long long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const int& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, int& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const short& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, short& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const char& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, char& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node,
                 unsigned long long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, unsigned long& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned int& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, unsigned int& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned short& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, unsigned short& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned char& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, unsigned char& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc, Entity payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(
        std::to_string(static_cast<std::underlying_type_t<Entity>>(payload))
            .c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Entity& payload) {
    std::underlying_type_t<Entity> numeric;
    Deserialize(node, numeric);
    payload = static_cast<Entity>(numeric);
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(payload ? "true" : "false");
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, bool& payload) {
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

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const double& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const float& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, double& payload) {
    try {
        double value = std::stod(node.value());
        payload = value;
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stod exception: {}, {}", e.what(), node.value());
    }
}

void Deserialize(const rapidxml::xml_node<>& node, float& payload) {
    try {
        float value = std::stof(node.value());
        payload = value;
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Vec2& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto x_attr = doc.allocate_attribute(
        "x", doc.allocate_string(std::to_string(payload.x).c_str()));
    auto y_attr = doc.allocate_attribute(
        "y", doc.allocate_string(std::to_string(payload.y).c_str()));
    node->append_attribute(x_attr);
    node->append_attribute(y_attr);
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Vec2& payload) {
    auto x_attr = node.first_attribute("x");
    auto y_attr = node.first_attribute("y");
    if (!x_attr || !y_attr) {
        LOGE("[Desrialize] parse Vec2 failed!, no x or y attribute");
        return;
    }

    try {
        payload.x = std::stof(x_attr->value());
        payload.y = std::stof(y_attr->value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, x = {}, y = {}", e.what(),
             x_attr->value(), y_attr->value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Region& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(doc, payload.m_topleft, "topleft"));
    node->append_node(Serialize(doc, payload.m_size, "size"));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Region& payload) {
    auto topleft_node = node.first_node("topleft");
    auto size_node = node.first_node("size");
    if (!topleft_node || !size_node) {
        LOGE("[Deserialize] parse Region failed! not topleft or size node");
        return;
    }
    Deserialize(*topleft_node, payload.m_topleft);
    Deserialize(*size_node, payload.m_size);
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Degrees& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload.Value()).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Degrees& payload) {
    try {
        payload = std::stof(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, value = {}", e.what(),
             node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Radians& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload.Value()).c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Radians& payload) {
    try {
        payload = std::stof(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stof exception: {}, value = {}", e.what(),
             node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Transform& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(doc, payload.m_position, "position"));
    node->append_node(Serialize(doc, payload.m_scale, "scale"));
    node->append_node(Serialize(doc, payload.m_rotation, "rotation"));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, Transform& payload) {
    auto position_node = node.first_node("position");
    auto scale_node = node.first_node("scale");
    auto rotation_node = node.first_node("rotation");

    if (!position_node || !scale_node || !rotation_node) {
        LOGE("[Deserialize] parse Pose failed! no position/scale/rotation "
             "field");
        return;
    }

    Deserialize(*position_node, payload.m_position);
    Deserialize(*scale_node, payload.m_scale);
    Deserialize(*rotation_node, payload.m_rotation);
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node, Handle<Image>& payload) {
    Path filename = node.value();
    auto& manager = GAME_CONTEXT.m_assets_manager->GetManager<Handle<T>>();
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
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

void Deserialize(const rapidxml::xml_node<>& node, Tilemap*& payload) {
    Path filename = node.value();
    auto& manager = GAME_CONTEXT.m_assets_manager->GetManager<TilemapHandle>();
    auto handle = manager.Load(filename);
    if (!handle) {
        handle = manager.Load(filename);
    }

    if (!handle) {
        payload = handle.Get();
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Handle<Tilemap> payload,
                                const std::string& name) {
    return Serialize(doc, &*payload, name);
}

void Deserialize(const rapidxml::xml_node<>& node, Handle<Tilemap>& payload) {
    Path filename = node.value();
    auto& manager = GAME_CONTEXT.m_assets_manager->GetManager<TilemapHandle>();
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const std::string& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(payload.c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, std::string& payload) {
    Path filename = node.value();
    payload = node.value();
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const UUID& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(payload.ToString().c_str()));
    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, UUID& payload) {
    payload = UUID::CreateFromString(node.value());
}

// x-macros for shorten code
#define HANDLE_ANIM_SERIALIZE(binding) if (binding_point == binding)
#define HANDLE_LINEAR_TRACK_SERIALIZE()                                \
    if (payload.GetType() == AnimationTrackType::Linear) {             \
        auto& keyframes = static_cast<const AnimationTrack<            \
            TARGET_TYPE, AnimationTrackType::Linear>&>(payload)        \
                              .GetKeyframes();                         \
        for (auto& keyframe : keyframes) {                             \
            auto keyframe_node = Serialize(doc, keyframe, "keyframe"); \
            keyframes_node->append_node(keyframe_node);                \
        }                                                              \
    }
#define HANDLE_DISCRETE_TRACK_SERIALIZE()                              \
    if (payload.GetType() == AnimationTrackType::Discrete) {           \
        auto& keyframes = static_cast<const AnimationTrack<            \
            TARGET_TYPE, AnimationTrackType::Discrete>&>(payload)      \
                              .GetKeyframes();                         \
        for (auto& keyframe : keyframes) {                             \
            auto keyframe_node = Serialize(doc, keyframe, "keyframe"); \
            keyframes_node->append_node(keyframe_node);                \
        }                                                              \
    }

rapidxml::xml_node<>* serializeAnimTrack(rapidxml::xml_document<>& doc,
                                         AnimationBindingPoint binding_point,
                                         const AnimationTrackBase& payload,
                                         const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(doc, binding_point, "binding_point"));
    auto keyframes_node =
        doc.allocate_node(rapidxml::node_type::node_element, "keyframes");
    node->append_node(Serialize(doc, payload.GetType(), "type"));
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

#define TARGET_TYPE Region
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteRegion) {
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_ANIM_SERIALIZE(AnimationBindingPoint::SpriteSize) {
        HANDLE_LINEAR_TRACK_SERIALIZE();
        HANDLE_DISCRETE_TRACK_SERIALIZE();
    }
#undef TARGET_TYPE

    return node;
}

#undef HANDLE_ANIM_SERIALIZE
#undef HANDLE_LINEAR_TRACK
#undef HANDLE_DISCRETE_TRACK

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Animation& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    auto tracks_node =
        doc.allocate_node(rapidxml::node_type::node_element, "tracks");

    auto& tracks = payload.GetTracks();
    for (auto& track : tracks) {
        auto track_node =
            serializeAnimTrack(doc, track.first, *track.second, "track");
        tracks_node->append_node(track_node);
    }

    node->append_node(tracks_node);
    return node;
}

#define HANDLE_ANIM_DESERIALIZE(binding) if (binding_point == binding)
#define HANDLE_CREATE_TRACK()                          \
    std::vector<KeyFrame<TARGET_TYPE>> keyframes;      \
    while (keyframe_node) {                            \
        KeyFrame<TARGET_TYPE> keyframe;                \
        Deserialize(*keyframe_node, keyframe);         \
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
deserializeTrack(rapidxml::xml_node<>& node) {
    AnimationBindingPoint binding_point = AnimationBindingPoint::Unknown;
    if (auto binding_point_node = node.first_node("binding_point")) {
        Deserialize(*binding_point_node, binding_point);
    }

    auto keyframes_node = node.first_node("keyframes");
    auto keyframe_node = keyframes_node->first_node("keyframe");
    AnimationTrackType type = AnimationTrackType::Discrete;
    auto type_node = node.first_node("type");
    Deserialize(*type_node, type);

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

#define TARGET_TYPE Region
    HANDLE_ANIM_DESERIALIZE(AnimationBindingPoint::SpriteRegion) {
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

    return {binding_point, std::move(track)};
}

#undef HANDLE_ANIM_DESERIALIZE
#undef HANDLE_LINEAR_TRACK_DESERIALIZE
#undef HANDLE_DISCRETE_TRACK_DESERIALIZE

void Deserialize(const rapidxml::xml_node<>& node, Animation& payload) {
    auto tracks_node = node.first_node("tracks");

    auto n = tracks_node->first_node();
    while (n) {
        auto [binding, track] = deserializeTrack(*n);
        payload.AddTrack(binding, std::move(track));
        n = n->next_sibling();
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const AnimationPlayer& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto loop_node = Serialize(doc, payload.GetLoopCount(), "loop");
    node->append_node(loop_node);

    auto rate_node = Serialize(doc, payload.GetRate(), "rate");
    node->append_node(rate_node);

    auto anim = payload.GetAnimation();
    if (anim) {
        auto anim_node = Serialize(doc, anim, "animation");
        node->append_node(anim_node);
    }

    return node;
}

void Deserialize(const rapidxml::xml_node<>& node, AnimationPlayer& payload) {
    if (auto loop_node = node.first_node("loop")) {
        int loop;
        Deserialize(*loop_node, loop);
        payload.SetLoop(loop);
    }

    if (auto anim_node = node.first_node("animation")) {
        AnimationHandle anim;
        Deserialize(*anim_node, anim);
        payload.ChangeAnimation(anim);
    }

    if (auto rate_node = node.first_node("rate")) {
        float rate;
        Deserialize(*rate_node, rate);
        payload.SetRate(rate);
    }
}