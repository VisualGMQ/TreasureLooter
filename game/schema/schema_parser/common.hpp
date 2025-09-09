#pragma once
#include "mustache.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

enum IncludeHint {
    None = 0,

    // std
    Array = 0x01,
    Option = 0x02,
    UnorderedMap = 0x04,
    Stdint = 0x08,

    // game related
    Handle = 0x10,
    Asset = 0x20,
};

struct PropertyInfo {
    std::string m_type;
    std::string m_name;
    bool m_optional = false;
    std::string m_default;
};

struct ClassInfo {
    std::string m_name;
    std::vector<PropertyInfo> m_properties;
    bool is_asset = false;
    std::string m_asset_extension;
};

struct EnumInfo {
    struct Item {
        std::string m_name;
        std::string m_value;
    };

    std::string m_name;
    std::string m_type;
    std::vector<Item> m_items;
};

struct SchemaInfo {
    int m_include_hints = None;
    
    std::vector<std::string> m_includes;
    std::filesystem::path m_filename;
    std::filesystem::path m_pure_filename; //! without directory & extension
    std::vector<ClassInfo> m_classes;
    std::vector<EnumInfo> m_enums;
    std::vector<std::string> m_imports;
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

    kainjow::mustache::mustache m_schema_serd_header_mustache;
    kainjow::mustache::mustache m_schema_serd_impl_mustache;
    kainjow::mustache::mustache m_enum_serd_header_mustache;
    kainjow::mustache::mustache m_enum_serd_impl_mustache;
    kainjow::mustache::mustache m_class_serd_header_mustache;
    kainjow::mustache::mustache m_class_serd_impl_mustache;
    kainjow::mustache::mustache m_asset_sl_header_mustache;
    kainjow::mustache::mustache m_asset_sl_impl_mustache;
    kainjow::mustache::mustache m_asset_extension_mustache;
    
    kainjow::mustache::mustache m_instance_display_header_mustache;
    kainjow::mustache::mustache m_instance_display_impl_mustache;
    kainjow::mustache::mustache m_enum_display_impl_mustache;
    kainjow::mustache::mustache m_enum_display_header_mustache;
    kainjow::mustache::mustache m_class_display_header_mustache;
    kainjow::mustache::mustache m_class_display_impl_mustache;

    static MustacheManager& GetInst();

private:
    MustacheManager();

    static kainjow::mustache::mustache readMustache(
        const std::filesystem::path& path);
};

std::string GetSchemaFileGenerateHeaderFilepath(const std::string& path);
std::string GetDisplayFileGenerateHeaderFilepath(const std::string& path);
std::string GetSerdFileGenerateHeaderFilepath(const std::string& path);
std::string GetSchemaFileGenerateImplFilepath(const std::string& path);
std::string GetDisplayFileGenerateImplFilepath(const std::string& path);
std::string GetSerdFileGenerateImplFilepath(const std::string& path);
