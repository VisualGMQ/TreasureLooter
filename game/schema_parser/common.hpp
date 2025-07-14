#pragma once
#include "mustache.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

enum STDLibs {
    None = 0,
    Array = 0x02,
    Option = 0x04,
    UnorderedMap = 0x08,
};

struct PropertyInfo {
    std::string m_type;
    std::string m_name;
    bool m_optional = false;
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
    int m_stb_lib_flag = None;
    
    std::vector<std::string> m_includes;
    std::filesystem::path m_filename;
    std::filesystem::path m_pure_filename; //! without directory & extension
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

    kainjow::mustache::mustache m_schema_serd_header_mustache;
    kainjow::mustache::mustache m_schema_serd_impl_mustache;
    kainjow::mustache::mustache m_enum_serd_header_mustache;
    kainjow::mustache::mustache m_enum_serd_impl_mustache;
    kainjow::mustache::mustache m_class_serd_header_mustache;
    kainjow::mustache::mustache m_class_serd_impl_mustache;

    static MustacheManager& GetInst();

private:
    MustacheManager();

    static kainjow::mustache::mustache readMustache(
        const std::filesystem::path& path);
};