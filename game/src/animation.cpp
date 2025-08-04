#include "animation.hpp"

#include "context.hpp"
#include "image.hpp"
#include "rapidxml_print.hpp"
#include "serialize.hpp"
#include "sprite.hpp"
#include "storage.hpp"

#include <sstream>

AnimationHandle AnimationManager::Load(const Path& filename) {
    if (auto handle = Find(filename)) {
        return handle;
    }

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
