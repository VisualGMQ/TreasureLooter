#pragma once
#include "SDL3/SDL.h"
#include "animation.hpp"
#include "asset.hpp"
#include "context.hpp"
#include "flag.hpp"
#include "handle.hpp"
#include "log.hpp"
#include "math.hpp"
#include "rapidxml.hpp"
#include "asset_manager.hpp"

#include <optional>
#include <string>

class Image;
class Tilemap;
class Relationship;

// integral
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const int& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const short& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const char& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned int& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned short& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned char& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, long long& payload);
void Deserialize(const rapidxml::xml_node<>& node, long& payload);
void Deserialize(const rapidxml::xml_node<>& node, int& payload);
void Deserialize(const rapidxml::xml_node<>& node, short& payload);
void Deserialize(const rapidxml::xml_node<>& node, char& payload);
void Deserialize(const rapidxml::xml_node<>& node, unsigned long long& payload);
void Deserialize(const rapidxml::xml_node<>& node, unsigned long& payload);
void Deserialize(const rapidxml::xml_node<>& node, unsigned int& payload);
void Deserialize(const rapidxml::xml_node<>& node, unsigned short& payload);
void Deserialize(const rapidxml::xml_node<>& node, unsigned char& payload);

// Path
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Path& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Path& payload);

// Entity
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                Entity payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Entity& payload);

// bool
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, bool& payload);

// floating
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const double& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const float& payload, const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, double& payload);
void Deserialize(const rapidxml::xml_node<>& node, float& payload);

// vec2
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Vec2& payload, const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Vec2& payload);

// region
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Region& payload, const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Region& payload);

// Degrees
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Degrees& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Degrees& payload);

// Radians
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Radians& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Radians& payload);

// transform
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Transform& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Transform& payload);

// std::string
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const std::string& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, std::string& payload);

// UUID
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const UUID& payload, const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, UUID& payload);

// Animation
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Animation& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, Animation& payload);

// AnimationPlayer
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const AnimationPlayer& payload,
                                const std::string& name);
void Deserialize(const rapidxml::xml_node<>& node, AnimationPlayer& payload);

// optional
template <typename T>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const std::optional<T>& payload,
                                const std::string& name) {
    if (!payload) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(doc, payload.value(), "value"));
    return node;
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node, std::optional<T>& payload) {
    auto value_node = node.first_node("value");
    if (!value_node) {
        payload = std::nullopt;
        return;
    }

    T value;
    Deserialize(*value_node, value);
    payload.emplace(std::move(value));
}

// vector
template <typename T>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const std::vector<T>& payload,
                                const std::string& name) {
    if (payload.empty()) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    for (auto& elem : payload) {
        auto elem_node = Serialize(doc, elem, "elem");
        node->append_node(elem_node);
    }
    return node;
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node, std::vector<T>& payload) {
    auto value_node = node.first_node("elem");
    while (value_node) {
        if (std::string_view{value_node->name()} != "elem") {
            continue;
        }

        T new_value;
        Deserialize(*value_node, new_value);
        payload.emplace_back(std::move(new_value));

        value_node = value_node->next_sibling();
    }
}

// array
template <typename T, size_t Size>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const std::array<T, Size>& payload,
                                const std::string& name) {
    if (payload.empty()) {
        return nullptr;
    }

    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));

    for (auto& elem : payload) {
        auto elem_node = Serialize(doc, elem, "elem");
        node->append_node(elem_node);
    }
    return node;
}

template <typename T, size_t Size>
void Deserialize(const rapidxml::xml_node<>& node,
                 std::array<T, Size>& payload) {
    auto value_node = node.first_node("elem");
    size_t size = Size;
    while (value_node && size > 0) {
        if (std::string_view{value_node->name()} != "elem") {
            continue;
        }

        Deserialize(*value_node, payload[Size - size]);

        value_node = value_node->next_sibling();
        size--;
    }
}

// unordered_map
template <typename Key, typename Value>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
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
        auto key_node = Serialize(doc, key, "key");
        auto value_node = Serialize(doc, value, "value");
        elem_node->append_node(key_node);
        elem_node->append_node(value_node);
        node->append_node(elem_node);
    }
    return node;
}

template <typename Key, typename Value>
void Deserialize(const rapidxml::xml_node<>& node,
                 std::unordered_map<Key, Value>& payload) {
    auto elem_node = node.first_node("elem");

    while (elem_node) {
        auto key_node = elem_node->first_node("key");
        auto value_node = elem_node->first_node("value");
        if (!key_node || !value_node) {
            LOGE("[Deserialize] parse unordered_map item failed: no key or "
                 "value node");
            continue;
        }

        Key key;
        Value value;
        Deserialize(*key_node, key);
        Deserialize(*value_node, value);
        payload.emplace(std::move(key), std::move(value));
    }
}

// AssetLoadResult
template <typename T>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const AssetLoadResult<T>& payload,
                                const std::string& name) {
    if (!payload.m_uuid) {
        LOGE("asset uuid is invalid! asset name: {}", name);
        return nullptr;
    }
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    auto uuid_node = Serialize(doc, payload.m_uuid, "uuid");
    node->append_node(uuid_node);

    auto value_node = Serialize(doc, payload.m_payload, "payload");
    node->append_node(value_node);
    return node;
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node,
                 AssetLoadResult<T>& payload) {
    auto uuid_node = node.first_node("uuid");
    Deserialize(*uuid_node, payload.m_uuid);

    auto value_node = node.first_node("payload");
    Deserialize(*value_node, payload.m_payload);
}

// Keyframe
template <typename T>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const KeyFrame<T>& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->append_node(Serialize(doc, payload.m_time, "time"));
    node->append_node(Serialize(doc, payload.m_value, "value"));
    return node;
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node, KeyFrame<T>& payload) {
    if (auto time_node = node.first_node("time")) {
        Deserialize(*time_node, payload.m_time);
    }
    if (auto value_node = node.first_node("value")) {
        Deserialize(*value_node, payload.m_value);
    }
}

// Handle<T>
template <typename T>
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                Handle<T> payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    if (payload) {
        auto filename = payload.GetFilename();
        if (filename) {
            node->value(doc.allocate_string(filename->string().c_str()));
        }
    }
    return node;
}

template <typename T>
void Deserialize(const rapidxml::xml_node<>& node, Handle<T>& payload) {
    Path filename = node.value();
    auto& manager =
        GAME_CONTEXT.m_assets_manager->GetManager<T>();
    payload = manager.Find(filename);
    if (!payload) {
        payload = manager.Load(filename);
    }
}
