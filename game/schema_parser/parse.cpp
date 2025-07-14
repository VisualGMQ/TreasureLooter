#include "parse.hpp"

#include <fstream>
#include <iostream>

std::optional<EnumInfo> parseEnum(rapidxml::xml_node<>* node) {
    EnumInfo info;
    auto name_node = node->first_attribute("name");
    if (!name_node) {
        std::cerr << "Error parsing enum, no name" << std::endl;
        return std::nullopt;
    }

    info.m_name = name_node->value();

    auto item_node = node->first_node("item");
    while (item_node) {
        EnumInfo::Item item;
        auto name = item_node->first_attribute("name");
        if (!name) {
            std::cerr << "Error parsing enum item, no name" << std::endl;
            continue;
        }
        item.m_name = name->value();
        auto value = item_node->first_attribute("value");
        if (value) {
            try {
                int num = std::stoi(value->value());
                item.m_value = num;
            } catch (std::invalid_argument const& ex) {
                std::cerr << "invalid argument when parse item value"
                          << ex.what() << std::endl;
            } catch (std::out_of_range const& ex) {
                std::cerr << "outof range when parse item value" << ex.what()
                          << std::endl;
            }
        }
        info.m_items.push_back(item);
        item_node = item_node->next_sibling("item");
    }

    return info;
}

std::optional<std::string> parseInclude(rapidxml::xml_node<>* node) {
    if (node->value() == nullptr) {
        return std::nullopt;
    }
    return node->value();
}

std::optional<PropertyInfo> parseElement(rapidxml::xml_node<>* node) {
    auto type = node->first_attribute("type");
    if (!type) {
        std::cerr << "Error parsing element, no type" << std::endl;
        return std::nullopt;
    }

    auto name = node->first_attribute("name");
    if (!type) {
        std::cerr << "Error parsing element, no name" << std::endl;
        return std::nullopt;
    }

    PropertyInfo property;
    property.m_name = name->value();
    property.m_type = type->value();
    return property;
}

std::optional<ClassInfo> parseClass(rapidxml::xml_node<>* node) {
    ClassInfo class_info;
    auto name_attr = node->first_attribute("name");
    if (!name_attr) {
        std::cerr << "Error parsing class, no name" << std::endl;
        return std::nullopt;
    }

    class_info.m_name = name_attr->value();

    auto element = node->first_node();
    while (element) {
        if (std::string_view{element->name()} != "element") {
            std::cerr << "Error parsing class, unknown node " << element->name()
                      << std::endl;
            return std::nullopt;
        }

        auto property = parseElement(element);
        if (property) {
            class_info.m_properties.push_back(property.value());
        }

        element = element->next_sibling();
    }

    return class_info;
}

std::optional<SchemaInfo> parseSchema(const std::filesystem::path& filename) {
    std::cout << "parsing " << filename << std::endl;
    rapidxml::xml_document<> doc;
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "Failed to open file " << filename << "\n";
        return std::nullopt;
    }

    std::string content(std::istreambuf_iterator<char>(file), {});

    doc.parse<0>(content.data());

    rapidxml::xml_node<>* node = doc.first_node();
    if (!node || std::string_view{node->name()} != "schema") {
        std::cerr << "Failed to parse schema\n";
        return std::nullopt;
    }

    SchemaInfo schema_info;
    schema_info.m_filename = filename;
    schema_info.m_pure_filename = filename.filename();
    schema_info.m_pure_filename =
        schema_info.m_pure_filename.replace_extension("");

    auto child = node->first_node();
    while (child) {
        std::string_view name = std::string_view{child->name()};
        if (name == "class") {
            auto class_info = parseClass(child);
            if (class_info) {
                schema_info.m_classes.push_back(class_info.value());
            }
        } else if (name == "include") {
            auto include_info = parseInclude(child);
            if (include_info) {
                schema_info.m_includes.push_back(include_info.value());
            }
        } else if (name == "enum") {
            auto info = parseEnum(child);
            if (info) {
                schema_info.m_enums.push_back(info.value());
            }
        }
        child = child->next_sibling();
    }

    return schema_info;
}