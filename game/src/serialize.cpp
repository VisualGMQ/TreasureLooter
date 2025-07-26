#include "serialize.hpp"

#include "context.hpp"
#include "image.hpp"
#include <stdexcept>

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(doc.allocate_string(std::to_string(payload).c_str()));
    return node;
}

void Deserialize(rapidxml::xml_node<>& node, long long& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, long& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, int& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, short& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, char& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, unsigned long long& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, unsigned long& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, unsigned int& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, unsigned short& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, unsigned char& payload) {
    try {
        payload = std::stoll(node.value());
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stoll exception: {}, {}", e.what(), node.value());
    }
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    node->value(payload ? "true" : "false");
    return node;
}

void Deserialize(rapidxml::xml_node<>& node, bool& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, double& payload) {
    try {
        double value = std::stod(node.value());
        payload = value;
    } catch (std::exception& e) {
        LOGE("[Deserialize]: stod exception: {}, {}", e.what(), node.value());
    }
}

void Deserialize(rapidxml::xml_node<>& node, float& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, Vec2& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, Region& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, Degrees& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, Radians& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, Transform& payload) {
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

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Image* payload, const std::string& name) {
    auto node = doc.allocate_node(rapidxml::node_type::node_element,
                                  doc.allocate_string(name.c_str()));
    if (payload) {
        node->value(doc.allocate_string(payload->Filename().string().c_str()));
    }
    return node;
}

rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Handle<Image> payload,
                                const std::string& name) {
    return Serialize(doc, &*payload, name);
}

void Deserialize(rapidxml::xml_node<>& node, Handle<Image>& payload) {
    Path filename = node.value();
    auto& image_manager = Context::GetInst().m_image_manager;
    payload = image_manager->Find(filename);
    if (!payload) {
        payload = image_manager->Load(filename);
    }
}

void Deserialize(rapidxml::xml_node<>& node, Image*& payload) {
    Path filename = node.value();
    auto& image_manager = Context::GetInst().m_image_manager;
    ImageHandle handle = image_manager->Load(filename);
    if (!handle) {
        handle = image_manager->Load(filename);
    }

    if (!handle) {
        payload = handle.Get();
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

void Deserialize(rapidxml::xml_node<>& node, std::string& payload) {
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

void Deserialize(rapidxml::xml_node<>& node, UUID& payload) {
    payload = UUID::CreateFromString(node.value());
}