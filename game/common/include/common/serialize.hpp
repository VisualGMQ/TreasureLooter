#pragma once
#include "common/context.hpp"
#include "common/animation.hpp"
#include "common/asset.hpp"
#include "common/asset_manager.hpp"
#include "common/flag.hpp"
#include "common/handle.hpp"
#include "common/log.hpp"
#include "common/math.hpp"
#include "rapidxml.hpp"
#include "schema/serialize/serialize.hpp"
#include "schema/asset_info.hpp"

#include <array>
#include <charconv>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

class Image;
class Tilemap;
class CollisionGroup;

// integral
rapidxml::xml_node<>* Serialize(CommonContext&, rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext&, rapidxml::xml_document<>& doc,
                                const long& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const int& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const short& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const char& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const unsigned long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const unsigned long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const unsigned int& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const unsigned short& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const unsigned char& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::byte& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, long long& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, long& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, int& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, short& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, char& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned long long& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned long& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned int& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned short& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, unsigned char& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, std::byte& payload);

// Path
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Path& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Path& payload);

// LogicEntity
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                LogicEntity payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, LogicEntity& payload);

// bool
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, bool& payload);

// floating
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const double& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const float& payload, const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, double& payload);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, float& payload);

// region
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Region& payload, const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Region& payload);

// Degrees
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Degrees& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Degrees& payload);

// Radians
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Radians& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Radians& payload);

// transform
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Transform& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Transform& payload);

// std::string
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::string& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, std::string& payload);

// UUID
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const UUIDv4& payload, const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, UUIDv4& payload);

// Animation
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Animation& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Animation& payload);

// collision group
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const CollisionGroup& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, CollisionGroup& payload);

// Color
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const Color& payload,
                                const std::string& name);
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Color& payload);

// `Serialize` returns nullptr for empty optionals, vectors, handles, ... and
// RapidXML's `append_node` writes through the argument, so linking a nullptr
// into the document corrupts it (or crashes). Every append goes through this.
inline void AppendNode(rapidxml::xml_node<>& parent,
                       rapidxml::xml_node<>* child) {
    if (child) {
        parent.append_node(child);
    }
}

// Floats are written with `std::to_chars` (shortest representation that
// round-trips), instead of `std::to_string` which truncates to 6 decimals and
// therefore loses precision on every save/load cycle. Integers keep
// `std::to_string`.
template <typename T>
std::string ToSerializedString(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        std::array<char, 32> buffer{};
        auto [ptr, ec] = std::to_chars(buffer.data(),
                                       buffer.data() + buffer.size(), value);
        if (ec == std::errc{}) {
            return std::string(buffer.data(),
                               static_cast<size_t>(ptr - buffer.data()));
        }
    }
    return std::to_string(value);
}

// optional
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::optional<T>& payload,
                                const std::string& name) {
    if (!payload) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    AppendNode(*node, Serialize(ctx,doc, payload.value(), "value"));
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, std::optional<T>& payload) {
    auto value_node = node.first_node("value");
    if (!value_node) {
        payload = std::nullopt;
        return;
    }

    T value;
    Deserialize(ctx, *value_node, value);
    payload.emplace(std::move(value));
}

// vector
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::vector<T>& payload,
                                const std::string& name) {
    if (payload.empty()) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    for (auto& elem : payload) {
        auto elem_node = Serialize(ctx,doc, elem, "elem");
        AppendNode(*node, elem_node);
    }
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, std::vector<T>& payload) {
    // Deserialization replaces the previous content: reloading an asset into an
    // object that already holds data used to append the old elements.
    payload.clear();
    // `next_sibling("elem")` instead of `next_sibling()`: a foreign sibling
    // would make the old `continue` branch spin forever.
    for (auto* value_node = node.first_node("elem"); value_node;
         value_node = value_node->next_sibling("elem")) {
        T new_value;
        Deserialize(ctx, *value_node, new_value);
        payload.emplace_back(std::move(new_value));
    }
}

// MatStorage
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const MatStorage<T>& payload,
                                const std::string& name) {
    if (payload.GetSize() == 0) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto w_node = Serialize(ctx, doc, payload.GetWidth(), "width");
    auto h_node = Serialize(ctx, doc, payload.GetHeight(), "height");
    AppendNode(*node, w_node);
    AppendNode(*node, h_node);

    for (size_t x = 0; x < payload.GetWidth(); x++) {
        for (size_t y = 0; y < payload.GetHeight(); y++) {
            auto elem_node = Serialize(ctx, doc, payload.Get(x, y), "elem");
            AppendNode(*node, elem_node);
        }
    }
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, MatStorage<T>& payload) {
    size_t w = 0, h = 0;
    if (auto w_node = node.first_node("width")) {
        Deserialize(ctx, *w_node, w);
    }
    if (auto h_node = node.first_node("height")) {
        Deserialize(ctx, *h_node, h);
    }
    payload.Resize(w, h);

    auto value_node = node.first_node("elem");
    size_t x = 0, y = 0;
    for (; value_node && x < w; value_node = value_node->next_sibling("elem")) {
        Deserialize(ctx, *value_node, payload.Get(x, y));
        y++;
        if (y >= h) {
            y = 0;
            x++;
        }
    }
}

// array
template <typename T, size_t Size>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::array<T, Size>& payload,
                                const std::string& name) {
    if (payload.empty()) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    for (auto& elem : payload) {
        auto elem_node = Serialize(ctx,doc, elem, "elem");
        AppendNode(*node, elem_node);
    }
    return node;
}

template <typename T, size_t Size>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node,
                 std::array<T, Size>& payload) {
    size_t size = Size;
    for (auto* value_node = node.first_node("elem"); value_node && size > 0;
         value_node = value_node->next_sibling("elem")) {
        Deserialize(ctx, *value_node, payload[Size - size]);
        size--;
    }
}

// unordered_map
template <typename Key, typename Value>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const std::unordered_map<Key, Value>& payload,
                                const std::string& name) {
    if (payload.empty()) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    for (auto&& [key, value] : payload) {
        auto elem_node =
            doc.allocate_node(rapidxml::node_type::node_element, "elem");
        auto key_node = Serialize(ctx,doc, key, "key");
        auto value_node = Serialize(ctx,doc, value, "value");
        AppendNode(*elem_node, key_node);
        AppendNode(*elem_node, value_node);
        AppendNode(*node, elem_node);
    }
    return node;
}

template <typename Key, typename Value>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node,
                 std::unordered_map<Key, Value>& payload) {
    // Deserialization replaces the previous content (see the vector overload).
    payload.clear();
    for (auto* elem_node = node.first_node("elem"); elem_node;
         elem_node = elem_node->next_sibling("elem")) {
        auto key_node = elem_node->first_node("key");
        auto value_node = elem_node->first_node("value");
        if (!key_node || !value_node) {
            LOGE("[Deserialize] parse unordered_map item failed: no key or "
                 "value node");
            continue;
        }

        Key key;
        Value value;
        Deserialize(ctx, *key_node, key);
        Deserialize(ctx, *value_node, value);
        payload.emplace(std::move(key), std::move(value));
    }
}

// AssetLoadResult
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const AssetLoadResult<T>& payload,
                                const std::string& name) {
    if (!payload.m_uuid) {
        LOGE("asset uuid is invalid! asset name: {}", name);
        return nullptr;
    }
    if (!payload.m_payload) {
        LOGE("asset payload is null! asset name: {}", name);
        return nullptr;
    }
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto uuid_node = Serialize(ctx,doc, payload.m_uuid, "uuid");
    AppendNode(*node, uuid_node);

    auto value_node = Serialize(ctx, doc, *payload.m_payload, "payload");
    AppendNode(*node, value_node);
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node,
                 AssetLoadResult<T>& payload) {
    auto uuid_node = node.first_node("uuid");
    if (!uuid_node) {
        LOGE("[Deserialize] {} node has no uuid", node.name());
        return;
    }
    Deserialize(ctx, *uuid_node, payload.m_uuid);

    auto value_node = node.first_node("payload");
    if (!value_node) {
        LOGE("[Deserialize] {} node has no payload", node.name());
        return;
    }
    payload.m_payload = std::make_unique<T>();
    Deserialize(ctx, *value_node, *payload.m_payload);
}

// Keyframe
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                const KeyFrame<T>& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    AppendNode(*node, Serialize(ctx,doc, payload.m_time, "time"));
    AppendNode(*node, Serialize(ctx,doc, payload.m_value, "value"));
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, KeyFrame<T>& payload) {
    if (auto time_node = node.first_node("time")) {
        Deserialize(ctx, *time_node, payload.m_time);
    }
    if (auto value_node = node.first_node("value")) {
        Deserialize(ctx, *value_node, payload.m_value);
    }
}

// Handle<T>
template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,rapidxml::xml_document<>& doc,
                                Handle<T> payload, const std::string& name) {
    TL_RETURN_VALUE_IF_FALSE(payload, {});
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    if constexpr (AssetSLInfo<T>::CanEmbed) {
        if (payload.IsEmbed()) {
            auto value_node = Serialize(
                COMMON_CONTEXT, doc, *payload, "payload");
            if (!value_node) {
                LOGE("save asset <embed> failed");
            }
            auto uuid_node = Serialize(COMMON_CONTEXT, doc, payload.GetUUID(), "uuid");
            if (!uuid_node) {
                LOGE("save asset <embed> failed");
            }

            AppendNode(*node, uuid_node);
            AppendNode(*node, value_node);
            return node;
        }
    }
    
    auto filename = payload.GetFilename();
    if (filename) {
        node->value(doc.allocate_string(filename->string().c_str()));
    }
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node, Handle<T>& payload) {
    auto& manager =
        static_cast<AssetManagerBase<T>&>(COMMON_CONTEXT.m_assets_manager->
            GetManager<T>());
    if (auto data_node = node.first_node(); data_node && data_node->type() == rapidxml::node_type::node_element) {
        if constexpr (AssetSLInfo<T>::CanEmbed) {
            TL_RETURN_IF_FALSE_WITH_LOG(data_node, LOGE,
                                        "[Asset]: node not has child node");
            AssetLoadResult<T> result = LoadAsset<T>(node);
            payload = manager.Create(result.m_uuid, std::move(result.m_payload),
                                     nullptr);
            return;
        } else {
            LOGE("[Asset]: asset {} can't embed", AssetInfoManager::GetName<T>());
            return;
        }
    }

    const char* value = node.value();
    if (!value) {
        LOGE("[Asset]: handle node has no value");
        return;
    }
    Path filename = value;
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}

// TVec2

template <typename T>
rapidxml::xml_node<>* Serialize(CommonContext& ctx,
                                rapidxml::xml_document<>& doc,
                                const TVec2<T>& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto x_attr = doc.allocate_attribute(
        "x", doc.allocate_string(ToSerializedString(payload.x).c_str()));
    auto y_attr = doc.allocate_attribute(
        "y", doc.allocate_string(ToSerializedString(payload.y).c_str()));
    node->append_attribute(x_attr);
    node->append_attribute(y_attr);
    return node;
}

template <typename T>
void Deserialize(CommonContext& ctx, const rapidxml::xml_node<>& node,
                 TVec2<T>& payload) {
    auto x_attr = node.first_attribute("x");
    auto y_attr = node.first_attribute("y");
    if (!x_attr || !y_attr) {
        LOGE("[Desrialize] parse TVec2<T> failed!, no x or y attribute");
        return;
    }

    try {
        payload.x = std::stod(x_attr->value());
        payload.y = std::stod(y_attr->value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stod exception: {}, x = {}, y = {}", e.what(),
             x_attr->value(), y_attr->value());
    }
}
