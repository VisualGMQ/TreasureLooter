#pragma once
#include "mustache.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct PropertyInfo {
    std::string m_type;
    std::string m_name;
};

struct ClassInfo {
    std::string m_name;
    std::vector<PropertyInfo> m_properties;
};

struct EnumInfo {
    struct Item {
        std::string m_name;
        std::optional<int> m_value;
    };
    
    std::string m_name;
    std::vector<Item> m_items;
};

struct SchemaInfo {
    std::vector<std::string> m_includes;
    std::filesystem::path m_filename;
    std::vector<ClassInfo> m_classes;
    std::vector<EnumInfo> m_enums;
};

struct SchemaInfoManager {
    std::vector<SchemaInfo> m_infos;
};

struct MustacheManager {
    kainjow::mustache::mustache m_class_mustache;
    kainjow::mustache::mustache m_property_mustache;
    kainjow::mustache::mustache m_schema_mustache;
    kainjow::mustache::mustache m_include_mustache;
    kainjow::mustache::mustache m_enum_mustache;

    static MustacheManager& GetInst();
    
private:
    MustacheManager();

    static kainjow::mustache::mustache readMustache(
        const std::filesystem::path& path);
};