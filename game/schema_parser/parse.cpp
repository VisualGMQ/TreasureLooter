#include "parse.hpp"

#include <fstream>
#include <iostream>

std::optional<EnumInfo> ParseEnum(rapidxml::xml_node<>* node) {
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
            item.m_value = value->value();
        }
        info.m_items.push_back(item);
        item_node = item_node->next_sibling("item");
    }

    return info;
}

std::optional<std::string> ParseInclude(rapidxml::xml_node<>* node) {
    if (node->value() == nullptr) {
        return std::nullopt;
    }
    return node->value();
}

std::optional<std::string> ParseImport(rapidxml::xml_node<>* node) {
    if (node->value() == nullptr) {
        return std::nullopt;
    }
    return node->value();
}

std::optional<PropertyInfo> ParseElement(rapidxml::xml_node<>* node) {
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

std::optional<PropertyInfo> ParseOption(SchemaInfo& schema,
                                        rapidxml::xml_node<>* node) {
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
    property.m_type = "std::optional<" + std::string{type->value()} + ">";
    property.m_optional = true;
    schema.m_stb_lib_flag = schema.m_stb_lib_flag | STDLibs::Option;
    return property;
}

std::optional<PropertyInfo> ParseArray(SchemaInfo& schema,
                                       rapidxml::xml_node<>* node) {
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

    bool is_dynamic = false;
    size_t count = 0;

    auto count_node = node->first_attribute("count");
    if (!count_node) {
        is_dynamic = true;
    } else {
        try {
            count = std::stoll(count_node->value());
        } catch (std::exception const& ex) {
            std::cerr << "Error parsing array, count invalid: "
                      << count_node->value() << std::endl;
        }
    }

    PropertyInfo property;
    property.m_name = name->value();
    if (is_dynamic) {
        property.m_type = "std::vector<" + std::string{type->value()} + ">";
    } else {
        property.m_type = "std::array<" + std::string{type->value()} + ", " +
                          std::to_string(count) + ">";
    }
    schema.m_stb_lib_flag = schema.m_stb_lib_flag | STDLibs::Array;
    return property;
}

std::optional<PropertyInfo> ParseUnorderedMap(SchemaInfo& schema,
                                              rapidxml::xml_node<>* node) {
    auto key_node = node->first_attribute("key");
    if (!key_node) {
        std::cerr << "Error parsing element, no key" << std::endl;
        return std::nullopt;
    }

    auto value_node = node->first_attribute("value");
    if (!value_node) {
        std::cerr << "Error parsing element, no value" << std::endl;
        return std::nullopt;
    }

    auto name_node = node->first_attribute("name");
    if (!name_node) {
        std::cerr << "Error parsing element, no name" << std::endl;
        return std::nullopt;
    }

    PropertyInfo property;
    property.m_name = name_node->value();
    property.m_type = std::string{"std::unordered_map<"} + key_node->value() +
                      ", " + value_node->value() + ">";
    schema.m_stb_lib_flag = schema.m_stb_lib_flag | STDLibs::UnorderedMap;
    return property;
}

std::optional<ClassInfo> ParseClass(SchemaInfo& schema,
                                    rapidxml::xml_node<>* node, bool is_asset) {
    ClassInfo class_info;
    class_info.is_asset = is_asset;
    auto name_attr = node->first_attribute("name");
    if (!name_attr) {
        std::cerr << "Error parsing class, no name" << std::endl;
        return std::nullopt;
    }

    if (is_asset) {
        auto extension_attr = node->first_attribute("extension");
        if (!extension_attr) {
            std::cerr << "Error parsing class, no extension" << std::endl;
            return std::nullopt;
        }
        class_info.m_asset_extension = extension_attr->value();
    }

    class_info.m_name = name_attr->value();

    auto element = node->first_node();
    while (element) {
        std::string_view name = element->name();
        if (name == "element") {
            auto property = ParseElement(element);
            if (property) {
                class_info.m_properties.push_back(property.value());
            }
        } else if (name == "option") {
            auto property = ParseOption(schema, element);
            if (property) {
                class_info.m_properties.push_back(property.value());
            }
        } else if (name == "array") {
            auto property = ParseArray(schema, element);
            if (property) {
                class_info.m_properties.push_back(property.value());
            }
        } else if (name == "unodered_map") {
            auto property = ParseUnorderedMap(schema, element);
            if (property) {
                class_info.m_properties.push_back(property.value());
            }
        } else {
            std::cerr << "Error parsing class, unknown node " << element->name()
                      << std::endl;
            return std::nullopt;
        }

        element = element->next_sibling();
    }

    return class_info;
}

std::optional<SchemaInfo> ParseSchema(const std::filesystem::path& filename) {
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
        if (name == "class" || name == "asset") {
            auto class_info = ParseClass(schema_info, child, name == "asset");
            if (class_info) {
                schema_info.m_classes.push_back(class_info.value());
            }
        } else if (name == "include") {
            auto include_info = ParseInclude(child);
            if (include_info) {
                schema_info.m_includes.push_back(include_info.value());
            }
        } else if (name == "import") {
            auto import_info = ParseImport(child);
            if (import_info) {
                schema_info.m_imports.push_back(import_info.value());
            }
        } else if (name == "enum") {
            auto info = ParseEnum(child);
            if (info) {
                schema_info.m_enums.push_back(info.value());
            }
        }
        child = child->next_sibling();
    }

    return schema_info;
}