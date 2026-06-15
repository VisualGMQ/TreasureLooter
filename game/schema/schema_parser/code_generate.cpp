#include "code_generate.hpp"
#include "common.hpp"
#include "mustache.hpp"
#include <cctype>
#include <unordered_map>
#include <unordered_set>

std::string toSnakeCase(std::string_view pascal) {
    if (pascal.empty()) return {};

    std::string result;
    result.reserve(pascal.size() + 4);

    for (size_t i = 0; i < pascal.size(); ++i) {
        char c = static_cast<char>(
            std::tolower(static_cast<unsigned char>(pascal[i])));
        if (i > 0 && std::isupper(static_cast<unsigned char>(pascal[i]))) {
            if (std::islower(static_cast<unsigned char>(pascal[i - 1])) ||
                (i + 1 < pascal.size() &&
                 std::islower(static_cast<unsigned char>(pascal[i + 1])))) {
                result += '_';
            }
        }
        result += c;
    }

    return result;
}

std::string toCamelCase(std::string_view name) {
    std::string snake = toSnakeCase(name);
    std::string result;
    result.reserve(snake.size());
    bool cap_next = true;
    for (char c : snake) {
        if (c >= 'a' && c <= 'z') {
            result += cap_next ? static_cast<char>(c - 'a' + 'A') : c;
            cap_next = false;
        } else if (c >= 'A' && c <= 'Z') {
            result += c;
            cap_next = false;
        } else if (c >= '0' && c <= '9') {
            result += c;
            cap_next = true;
        } else {
            cap_next = true;
        }
    }
    return result;
}

std::string extractOptionalInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::optional<";
    if (cpp_type.size() <= prefix.size() + 1 ||
        cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<')
            ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0)
                return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

std::string extractFlagsInnerType(const std::string& cpp_type) {
    const std::string prefix = "Flags<";
    if (cpp_type.size() <= prefix.size() + 1 ||
        cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<')
            ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0)
                return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

std::string extractVectorInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::vector<";
    if (cpp_type.size() <= prefix.size() + 1 ||
        cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<')
            ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0)
                return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

std::string extractUnorderedMapInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::unordered_map<";
    if (cpp_type.size() <= prefix.size() + 1 ||
        cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

std::string extractArrayInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::array<";
    if (cpp_type.size() <= prefix.size() + 1 ||
        cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    size_t comma = cpp_type.find(',', prefix.size());
    if (comma == std::string::npos) return {};
    return cpp_type.substr(prefix.size(), comma - prefix.size());
}

std::string GenerateClassCode(const ClassInfo& info) {
    kainjow::mustache::data prop_datas{kainjow::mustache::data::type::list};

    auto& prop_mustache = MustacheManager::GetInst().m_property_mustache;
    for (auto& property : info.m_properties) {
        kainjow::mustache::data prop_data;
        prop_data.set("type", property.m_type);
        prop_data.set("name", property.m_name);
        if (!property.m_default.empty()) {
            prop_data.set("default", "= " + property.m_default);
        }

        auto prop_code = prop_mustache.render(prop_data);
        prop_datas << kainjow::mustache::data{"property", prop_code};
    }

    kainjow::mustache::data class_data;
    class_data.set("class_name", info.m_name);
    class_data.set("properties", prop_datas);

    if (info.is_asset) {
        class_data.set("is_asset", true);
    }

    auto& class_mustache = MustacheManager::GetInst().m_class_mustache;
    return class_mustache.render(class_data);
}

std::string GenerateSchemaCode(const SchemaInfo& schema_info) {
    auto& schema_mustache = MustacheManager::GetInst().m_schema_mustache;
    auto& include_mustache = MustacheManager::GetInst().m_include_mustache;

    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data include_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data import_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};

    for (auto& include : schema_info.m_includes) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "\"" + include + "\""})};
    }

    if (schema_info.m_include_hints & IncludeHint::Option) {
        include_datas << kainjow::mustache::data{
            "include", include_mustache.render({"filename", "<optional>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::Flags) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render(
                {"filename", "\"common/script/script_flags_binding.hpp\""})};
    }
    if (schema_info.m_include_hints & IncludeHint::Array) {
        include_datas << kainjow::mustache::data{"include",
                                                 include_mustache.render(
                                                     {"filename", "<array>"})}
                      << kainjow::mustache::data{
                             "include",
                             include_mustache.render({"filename", "<vector>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::UnorderedMap) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "<unordered_map>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::Stdint) {
        include_datas << kainjow::mustache::data{
            "include", include_mustache.render({"filename", "<cstdint>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::Handle) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "\"common/handle.hpp\""})};
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render(
                {"filename", "\"common/script/script_handle_binding.hpp\""})};
    }

    for (auto& import_filename : schema_info.m_imports) {
        import_datas << kainjow::mustache::data{
            "import_filename",
            GetSchemaFileGenerateHeaderFilepath(import_filename)};
    }

    for (auto& clazz : schema_info.m_classes) {
        std::string code = GenerateClassCode(clazz);
        class_datas << kainjow::mustache::data{"class", code};
    }

    for (auto& enum_info : schema_info.m_enums) {
        std::string code = GenerateEnumCode(enum_info);
        enum_datas << kainjow::mustache::data{"enum", code};
    }

    kainjow::mustache::data final_datas;
    final_datas.set("classes", class_datas);
    final_datas.set("includes", include_datas);
    final_datas.set("enums", enum_datas);
    final_datas.set("import_filenames", import_datas);
    return schema_mustache.render(final_datas);
}

std::string GenerateEnumCode(const EnumInfo& enum_info) {
    auto& enum_mustache = MustacheManager::GetInst().m_enum_mustache;

    kainjow::mustache::data item_datas{kainjow::mustache::data::type::list};

    for (auto& item : enum_info.m_items) {
        std::string item_declare = item.m_name;
        if (!item.m_value.empty()) {
            item_declare += " = " + item.m_value;
        }

        item_datas << kainjow::mustache::data{"item", item_declare};
    }

    kainjow::mustache::data enum_data;
    enum_data.set("items", item_datas);
    enum_data.set("name", enum_info.m_name);
    enum_data.set("count", std::to_string(enum_info.m_items.size()));

    if (!enum_info.m_type.empty()) {
        enum_data.set("type", " : " + enum_info.m_type);
    }

    return enum_mustache.render(enum_data);
}

std::string GenerateSchemaSerializeHeaderCode(const SchemaInfo& schema) {
    auto& header_mustache =
        MustacheManager::GetInst().m_schema_serd_header_mustache;

    kainjow::mustache::data data;
    kainjow::mustache::data include_datas{kainjow::mustache::data::type::list};

    for (auto& include : schema.m_includes) {
        include_datas << kainjow::mustache::data{"include", include};
    }

    auto generate_header_filename = schema.m_pure_filename;
    generate_header_filename =
        generate_header_filename.replace_extension(".hpp");
    auto final_path = "schema/" + generate_header_filename.string();
    if (schema.m_include_hints & IncludeHint::Asset) {
        include_datas << kainjow::mustache::data{"include", "common/asset.hpp"};
    }
    include_datas << kainjow::mustache::data{"include", final_path.c_str()};
    data.set("includes", include_datas);

    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data asset_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumSerializeHeaderCode(enum_value);
        enum_datas << kainjow::mustache::data{"enum", code};
    }

    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassSerializeHeaderCode(class_value);
        class_datas << kainjow::mustache::data{"class", code};

        if (class_value.is_asset) {
            auto asset_code = GenerateAssetSLHeaderCode(class_value);
            asset_datas << kainjow::mustache::data{"asset_sl", asset_code};
        }
    }

    data.set("enums", enum_datas);
    data.set("classes", class_datas);
    data.set("assets_sl", asset_datas);

    return header_mustache.render(data);
}

std::string GenerateSchemaSerializeImplCode(const SchemaInfo& schema) {
    auto& mustache = MustacheManager::GetInst().m_schema_serd_impl_mustache;

    kainjow::mustache::data data;
    data.set("filename", schema.m_pure_filename.string().c_str());

    kainjow::mustache::data import_datas{kainjow::mustache::data::type::list};
    for (auto& import : schema.m_imports) {
        auto filepath = GetSerdFileGenerateHeaderFilepath(import);
        import_datas << kainjow::mustache::data{"import_filename", filepath};
    }
    if (schema.m_include_hints & IncludeHint::Asset) {
        import_datas << kainjow::mustache::data{"import_filename",
                                                "common/asset.hpp"};
    }
    data.set("import_filenames", import_datas);

    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumSerializeImplCode(enum_value);
        enum_datas << kainjow::mustache::data{"enum_impl", code};
    }

    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data asset_datas{kainjow::mustache::data::type::list};
    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassSerializeImplCode(class_value);
        class_datas << kainjow::mustache::data{"class_impl", code};

        if (class_value.is_asset) {
            auto asset_code = GenerateAssetSLImplCode(class_value);
            asset_datas << kainjow::mustache::data{"asset_sl", asset_code};
        }
    }

    data.set("enum_impls", enum_datas);
    data.set("class_impls", class_datas);
    data.set("assets_sl", asset_datas);

    return mustache.render(data);
}

std::string GenerateEnumSerializeHeaderCode(const EnumInfo& enum_info) {
    auto& mustache = MustacheManager::GetInst().m_enum_serd_header_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);

    return mustache.render(data);
}

std::string GenerateEnumSerializeImplCode(const EnumInfo& enum_info) {
    auto& mustache = MustacheManager::GetInst().m_enum_serd_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);

    kainjow::mustache::data branches_data{kainjow::mustache::data::type::list};
    for (auto& item : enum_info.m_items) {
        branches_data << kainjow::mustache::data{"enum_item", item.m_name};
    }
    data.set("branches", branches_data);

    return mustache.render(data);
}

std::string GenerateClassSerializeHeaderCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_serd_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    if (info.is_asset) {
        data.set("has_handle", true);
    }

    return mustache.render(data);
}

std::string GenerateClassSerializeImplCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_serd_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    kainjow::mustache::data properties_data{
        kainjow::mustache::data::type::list};
    for (auto& prop : info.m_properties) {
        kainjow::mustache::data property_data;
        property_data.set("property", prop.m_name);
        property_data.set("is_optional", prop.m_is_optional ? "true" : "false");
        property_data.set("is_handle", prop.m_is_handle ? "true" : "false");
        property_data.set("is_array", prop.m_is_array ? "true" : "false");
        properties_data << property_data;
    }

    data.set("properties", properties_data);
    if (info.is_asset) {
        data.set("has_handle", true);
    }

    return mustache.render(data);
}

std::string GenerateAssetSLHeaderCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_asset_sl_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    return mustache.render(data);
}

std::string GenerateAssetSLImplCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_asset_sl_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    return mustache.render(data);
}

void GenerateSchemaAssetExtensionMustacheData(
    const SchemaInfo& schema, kainjow::mustache::data& out_data) {
    for (auto& clazz : schema.m_classes) {
        if (!clazz.is_asset) {
            continue;
        }

        kainjow::mustache::data ext_data;
        ext_data.set("type", clazz.m_name);
        ext_data.set("extension", clazz.m_asset_extension);

        out_data << ext_data;
    }
}

std::string GenerateEnumDisplayHeaderCode(const EnumInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_enum_display_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    return mustache.render(data);
}

std::string GenerateEnumDisplayImplCode(const EnumInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_enum_display_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    kainjow::mustache::data items_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data enum_to_idx_list{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data idx_to_enum_list{
        kainjow::mustache::data::type::list};
    for (size_t i = 0; i < info.m_items.size(); i++) {
        auto& item = info.m_items[i];
        items_datas << kainjow::mustache::data{"item", item.m_name};
        kainjow::mustache::data enum_to_idx;
        enum_to_idx.set("type", info.m_name);
        enum_to_idx.set("item", item.m_name);
        enum_to_idx.set("idx", std::to_string(i));
        enum_to_idx_list << enum_to_idx;
        idx_to_enum_list << enum_to_idx;
    }

    data.set("items", items_datas);
    data.set("enum_to_idx_list", enum_to_idx_list);
    data.set("idx_to_enum_list", idx_to_enum_list);

    return mustache.render(data);
}

std::string GenerateClassDisplayHeaderCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_display_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    if (info.is_asset) {
        data.set("has_handle", true);
    }

    return mustache.render(data);
}

std::string GenerateClassDisplayImplCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_display_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    kainjow::mustache::data properties_data{
        kainjow::mustache::data::type::list};
    for (auto& prop : info.m_properties) {
        kainjow::mustache::data property_data;
        property_data.set("property", prop.m_name);
        properties_data << property_data;
    }

    data.set("has_handle", info.is_asset);

    data.set("type_extension", info.m_asset_extension_var);

    data.set("properties", properties_data);

    return mustache.render(data);
}

std::string GenerateSchemaDisplayHeaderCode(const SchemaInfo& schema) {
    auto& header_mustache =
        MustacheManager::GetInst().m_instance_display_header_mustache;

    kainjow::mustache::data data;
    kainjow::mustache::data include_datas{kainjow::mustache::data::type::list};

    for (auto& include : schema.m_includes) {
        include_datas << kainjow::mustache::data{"include", include};
    }

    auto generate_header_filename = schema.m_pure_filename;
    generate_header_filename =
        generate_header_filename.replace_extension(".hpp");
    auto final_path = "schema/" + generate_header_filename.string();
    include_datas << kainjow::mustache::data{"include", final_path};
    data.set("includes", include_datas);

    kainjow::mustache::data display_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumDisplayHeaderCode(enum_value);
        display_datas << kainjow::mustache::data{"display", code};
    }

    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassDisplayHeaderCode(class_value);
        display_datas << kainjow::mustache::data{"display", code};
    }

    data.set("displays", display_datas);

    return header_mustache.render(data);
}

std::string GenerateSchemaDisplayImplCode(const SchemaInfo& schema) {
    auto& mustache =
        MustacheManager::GetInst().m_instance_display_impl_mustache;

    kainjow::mustache::data data;
    data.set("filename", schema.m_pure_filename.string().c_str());

    kainjow::mustache::data import_datas{kainjow::mustache::data::type::list};
    for (auto& import : schema.m_imports) {
        import_datas << kainjow::mustache::data{
            "import_filename", GetDisplayFileGenerateHeaderFilepath(import)};
    }
    data.set("import_filenames", import_datas);

    kainjow::mustache::data display_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumDisplayImplCode(enum_value);
        display_datas << kainjow::mustache::data{"display_impl", code};
    }

    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassDisplayImplCode(class_value);
        display_datas << kainjow::mustache::data{"display_impl", code};
    }

    data.set("display_impls", display_datas);

    return mustache.render(data);
}

const std::unordered_map<std::string, std::string> CppTypeToProtoType = {
    {          "double", "double"},
    {           "float",  "float"},
    {             "int",  "int32"},
    {            "long",  "int64"},
    {        "uint32_t", "uint32"},
    {            "bool",   "bool"},
    {     "std::string", "string"},
    {"std::string_view", "string"},
};

const char* ProtoRepeatedKeyword = "repeated";

const std::unordered_map<std::string, std::string> ProtoScalarToCppType = {
    {  "float",  "float"},
    { "double", "double"},
    {  "int32", "::int32_t"},
    {  "int64", "::int64_t"},
    { "uint32", "::uint32_t"},
    {   "bool",   "bool"},
};

std::string protoScalarToCppType(const std::string& proto_type) {
    auto it = ProtoScalarToCppType.find(proto_type);
    if (it != ProtoScalarToCppType.end()) {
        return it->second;
    }
    return proto_type;
}

std::string typeConvertToProto(const std::string& type) {
    if (auto it = CppTypeToProtoType.find(type);
        it != CppTypeToProtoType.end()) {
        return it->second;
    }
    return type;
}

std::string GenerateProtoClassDeclareCode(const ClassInfo& info) {
    assert(info.ShouldGenProto());

    auto& mustache = MustacheManager::GetInst().m_proto_class_declare_mustache;
    kainjow::mustache::data data;

    data.set("type", info.m_name);

    kainjow::mustache::data fields_data{kainjow::mustache::data::type::list};
    for (auto& property : info.m_properties) {
        kainjow::mustache::data field_data;

        field_data.set("field_name", property.m_name);
        if (property.m_proto_id) {
            field_data.set("field_id",
                           std::to_string(property.m_proto_id.value()));
        }
        if (property.m_is_array) {
            field_data.set("repeated", ProtoRepeatedKeyword);
            field_data.set("field_type",
                           typeConvertToProto(property.m_template_type1));
        } else if (property.m_is_flags) {
            // TODO: use enum underlying type
            field_data.set("field_type", "uint32");
        } else if (property.m_is_optional) {
            field_data.set("field_type",
                           typeConvertToProto(property.m_template_type1));
        } else if (property.m_is_unordered_map) {
            field_data.set(
                "field_type",
                "map<" + typeConvertToProto(property.m_template_type1) + ", " +
                    typeConvertToProto(property.m_template_type2) + ">");
        } else {
            field_data.set("field_type", typeConvertToProto(property.m_type));
        }

        fields_data << field_data;
    }

    data.set("fields", fields_data);

    return mustache.render(data);
}

std::string GenerateProtoEnumDeclareCode(const EnumInfo& info) {
    assert(info.ShouldGenProto());

    auto& mustache = MustacheManager::GetInst().m_proto_enum_declare_mustache;
    kainjow::mustache::data data;
    data.set("type", info.m_type);

    kainjow::mustache::data items{kainjow::mustache::data::type::list};
    for (auto& item : info.m_items) {
        kainjow::mustache::data item_data;
        item_data.set("item", item.m_name);
        item_data.set("value", item.m_value);
        items << item_data;
    }
    data.set("items", items);

    return mustache.render(data);
}

std::string GenerateProtoSchemaDeclareCode(const SchemaInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_proto_mustache;
    kainjow::mustache::data data;

    kainjow::mustache::data imports_data{kainjow::mustache::data::type::list};
    for (auto& import : info.m_imports) {
        kainjow::mustache::data import_data;
        import_data.set("import", import + ".proto");
        imports_data << import_data;
    }

    kainjow::mustache::data enums_data{kainjow::mustache::data::type::list};
    for (auto& enum_info : info.m_enums) {
        if (!enum_info.ShouldGenProto()) {
            continue;
        }

        kainjow::mustache::data enum_data;
        enum_data.set("enum", GenerateProtoEnumDeclareCode(enum_info));

        enums_data << enum_data;
    }

    kainjow::mustache::data classes_data{kainjow::mustache::data::type::list};
    for (auto& class_info : info.m_classes) {
        if (!class_info.ShouldGenProto()) {
            continue;
        }

        kainjow::mustache::data class_data;
        class_data.set("class", GenerateProtoClassDeclareCode(class_info));

        classes_data << class_data;
    }

    data.set("imports", imports_data);
    data.set("enums", enums_data);
    data.set("classes", classes_data);

    return mustache.render(data);
}

std::string GenerateProtoAllProtoDeclareCode(const SchemaInfoManager& mgr) {
    auto& mustache = MustacheManager::GetInst().m_all_proto_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data import_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data oneof_classes_data{
        kainjow::mustache::data::type::list};

    bool has_proto = false;
    for (auto& schema_info : mgr.m_infos) {
        std::filesystem::path proto_filename = schema_info.m_pure_filename;
        proto_filename.replace_extension("proto");

        bool schema_has_proto = false;
        for (auto& enum_info : schema_info.m_enums) {
            if (!enum_info.m_proto_id) {
                continue;
            }

            schema_has_proto = true;

            kainjow::mustache::data oneof_class_data;
            oneof_class_data.set("class_type", enum_info.m_name);
            oneof_class_data.set("class_name_snake_case",
                                 toSnakeCase(enum_info.m_name));
            oneof_class_data.set("proto_id",
                                 std::to_string(enum_info.m_proto_id.value()));

            oneof_classes_data << oneof_class_data;
        }

        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.m_proto_id) {
                continue;
            }

            schema_has_proto = true;

            kainjow::mustache::data oneof_class_data;
            oneof_class_data.set("class_type", class_info.m_name);
            oneof_class_data.set("class_name_snake_case",
                                 toSnakeCase(class_info.m_name));
            oneof_class_data.set("proto_id",
                                 std::to_string(class_info.m_proto_id.value()));

            oneof_classes_data << oneof_class_data;
        }

        if (schema_has_proto) {
            kainjow::mustache::data import_data;
            import_data.set("import", proto_filename.string());
            import_datas << import_data;

            has_proto = true;
        }
    }

    if (has_proto) {
        datas.set("imports", import_datas);
        datas.set("has_oneof", true);
        datas.set("oneof_classes", oneof_classes_data);
    }

    return mustache.render(datas);
}

std::string GenerateProtoNetMsgDispatchHeaderCode(const SchemaInfoManager&) {
    auto& mustache = MustacheManager::GetInst().m_net_msg_dispatch_header_mustache;
    kainjow::mustache::data data;
    return mustache.render(data);
}

std::string GenerateProtoNetMsgDispatchImplCode(const SchemaInfoManager& mgr) {
    auto& mustache = MustacheManager::GetInst().m_net_msg_dispatch_impl_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data msg_datas{kainjow::mustache::data::type::list};

    for (auto& schema_info : mgr.m_infos) {
        for (auto& enum_info : schema_info.m_enums) {
            if (!enum_info.m_proto_id) {
                continue;
            }

            kainjow::mustache::data msg_data;
            msg_data.set("msg_camel_case_name", toCamelCase(enum_info.m_name));
            std::string name = toSnakeCase(enum_info.m_name);
            msg_data.set("msg_snake_case_name", name);

            msg_datas << msg_data;
        }

        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.m_proto_id) {
                continue;
            }

            kainjow::mustache::data msg_data;
            msg_data.set("msg_camel_case_name", toCamelCase(class_info.m_name));
            std::string name = toSnakeCase(class_info.m_name);
            msg_data.set("msg_snake_case_name", name);

            msg_datas << msg_data;
        }
    }

    datas.set("msgs", msg_datas);

    return mustache.render(datas);
}

std::string GenerateProtoBindingHeaderCode(const SchemaInfoManager&) {
    auto& mustache = MustacheManager::GetInst().m_proto_binding_header_mustache;
    kainjow::mustache::data data;
    return mustache.render(data);
}

std::string GenerateProtoBindingImplCode(const SchemaInfoManager& mgr) {
    auto& mustache = MustacheManager::GetInst().m_proto_binding_impl_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data classes_data{kainjow::mustache::data::type::list};

    for (auto& schema_info : mgr.m_infos) {
        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.ShouldGenProto()) {
                continue;
            }

            kainjow::mustache::data class_data;
            class_data.set("class_name", class_info.m_name);
            class_data.set("class_full_name",
                           "proto::" + class_info.m_name);
            class_data.set("has_schema", true);

            kainjow::mustache::data fields_data{
                kainjow::mustache::data::type::list};
            for (auto& property : class_info.m_properties) {
                if (!property.m_proto_id) {
                    continue;
                }

                kainjow::mustache::data field_data;
                field_data.set("field_name", "m_" + property.m_name);

                bool is_builtin =
                    CppTypeToProtoType.find(property.m_type) !=
                    CppTypeToProtoType.end();
                bool is_string = property.m_type == "std::string" ||
                                 property.m_type == "std::string_view";

                if (!is_builtin) {
                    field_data.set("is_message", true);
                    field_data.set("field_type",
                                   "::proto::" + property.m_type);
                    field_data.set("has_has", true);
                } else if (is_string) {
                    field_data.set("is_string", true);
                } else {
                    field_data.set("is_scalar", true);
                    auto it = CppTypeToProtoType.find(property.m_type);
                    field_data.set("field_type",
                                   protoScalarToCppType(it->second));
                }
                fields_data << field_data;
            }
            class_data.set("fields", fields_data);

            classes_data << class_data;
        }
    }

    // NetMsg oneof binding
    {
        kainjow::mustache::data net_msg_data;
        net_msg_data.set("class_name", std::string{"NetMsg"});
        net_msg_data.set("class_full_name",
                         std::string{"proto::NetMsg"});

        kainjow::mustache::data net_msg_fields{
            kainjow::mustache::data::type::list};
        for (auto& schema_info : mgr.m_infos) {
            for (auto& class_info : schema_info.m_classes) {
                if (!class_info.m_proto_id) {
                    continue;
                }
                kainjow::mustache::data field_data;
                field_data.set("field_name",
                               "m_" + toSnakeCase(class_info.m_name));
                field_data.set("is_message", true);
                field_data.set("field_type",
                               "::proto::" + class_info.m_name);
                field_data.set("has_has", true);
                net_msg_fields << field_data;
            }
            for (auto& enum_info : schema_info.m_enums) {
                if (!enum_info.m_proto_id) {
                    continue;
                }
                kainjow::mustache::data field_data;
                field_data.set("field_name",
                               "m_" + toSnakeCase(enum_info.m_name));
                field_data.set("is_message", true);
                field_data.set("field_type",
                               "::proto::" + enum_info.m_name);
                field_data.set("has_has", true);
                net_msg_fields << field_data;
            }
        }
        net_msg_data.set("fields", net_msg_fields);

        classes_data << net_msg_data;
    }

    datas.set("classes", classes_data);

    return mustache.render(datas);
}

std::string GenerateProtoEventBindingHeaderCode(const SchemaInfoManager&) {
    auto& mustache =
        MustacheManager::GetInst().m_proto_event_binding_header_mustache;
    kainjow::mustache::data data;
    return mustache.render(data);
}

std::string GenerateProtoEventBindingImplCode(const SchemaInfoManager& mgr) {
    auto& mustache =
        MustacheManager::GetInst().m_proto_event_binding_impl_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data events_data{kainjow::mustache::data::type::list};

    for (auto& schema_info : mgr.m_infos) {
        for (auto& enum_info : schema_info.m_enums) {
            if (!enum_info.m_proto_id) {
                continue;
            }
            kainjow::mustache::data event_data;
            event_data.set("event_type", enum_info.m_name);
            event_data.set("event_name", enum_info.m_name);
            events_data << event_data;
        }

        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.m_proto_id) {
                continue;
            }
            kainjow::mustache::data event_data;
            event_data.set("event_type", class_info.m_name);
            event_data.set("event_name", class_info.m_name);
            events_data << event_data;
        }
    }

    datas.set("events", events_data);

    return mustache.render(datas);
}

std::string GenerateProtoConvertHeaderCode(const SchemaInfoManager& mgr) {
    auto& mustache =
        MustacheManager::GetInst().m_proto_convert_header_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data includes_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data conversions_data{
        kainjow::mustache::data::type::list};

    for (auto& schema_info : mgr.m_infos) {
        bool has_proto_class = false;
        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.ShouldGenProto()) {
                continue;
            }
            has_proto_class = true;

            kainjow::mustache::data conv_data;
            conv_data.set("class_name", class_info.m_name);
            conversions_data << conv_data;
        }
        if (has_proto_class) {
            includes_data << kainjow::mustache::data{
                "include",
                std::string{"schema/"} +
                    schema_info.m_pure_filename.string() + ".hpp"};
        }
    }

    datas.set("includes", includes_data);
    datas.set("conversions", conversions_data);

    return mustache.render(datas);
}

std::string GenerateProtoConvertImplCode(const SchemaInfoManager& mgr) {
    auto& mustache =
        MustacheManager::GetInst().m_proto_convert_impl_mustache;
    kainjow::mustache::data datas;
    kainjow::mustache::data includes_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data conversions_data{
        kainjow::mustache::data::type::list};

    for (auto& schema_info : mgr.m_infos) {
        bool has_proto_class = false;
        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.ShouldGenProto()) {
                continue;
            }
            has_proto_class = true;

            kainjow::mustache::data conv_data;
            conv_data.set("class_name", class_info.m_name);

            kainjow::mustache::data fields_data{
                kainjow::mustache::data::type::list};
            for (auto& property : class_info.m_properties) {
                if (!property.m_proto_id) {
                    continue;
                }

                kainjow::mustache::data field_data;
                field_data.set("field_name", "m_" + property.m_name);
                field_data.set(
                    "is_message",
                    CppTypeToProtoType.find(property.m_type) ==
                        CppTypeToProtoType.end());
                fields_data << field_data;
            }
            conv_data.set("fields", fields_data);

            conversions_data << conv_data;
        }
        if (has_proto_class) {
            includes_data << kainjow::mustache::data{
                "include",
                std::string{"schema/"} +
                    schema_info.m_pure_filename.string() + ".hpp"};
        }
    }

    datas.set("includes", includes_data);
    datas.set("conversions", conversions_data);

    return mustache.render(datas);
}

std::string GenerateAssetInfoHeaderCode(const SchemaInfoManager& manager) {
    kainjow::mustache::data data;
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data extensions_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data names_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data type_checks_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data extensions_data2{
        kainjow::mustache::data::type::list};

    std::vector<std::string> type_names;
    int asset_num = 0;
    for (auto& info : manager.m_infos) {
        bool included = false;
        for (auto& clazz : info.m_classes) {
            if (!clazz.is_asset) {
                continue;
            }

            asset_num++;

            if (!included) {
                includes_data << kainjow::mustache::data{
                    "include", info.m_generate_filename.string()};
                included = true;
            }

            type_names.push_back(clazz.m_name);

            kainjow::mustache::data extension_data;
            extension_data.set("extension", clazz.m_asset_extension);
            extension_data.set("extension_var", clazz.m_asset_extension_var);
            extensions_data << extension_data;

            kainjow::mustache::data extension_data2;
            extension_data2.set("extension_var", clazz.m_asset_extension_var);
            extensions_data2 << extension_data2;

            kainjow::mustache::data type_check_data;
            type_check_data.set("type", clazz.m_name);
            type_check_data.set("extension_var", clazz.m_asset_extension_var);
            type_checks_data << type_check_data;
        }

        for (auto& cpp_def : info.m_cpp_asset_defs) {
            auto extension_var = cpp_def.m_asset_name +
                                 std::string{ClassInfo::ExtensionVarSuffix};

            type_names.push_back(cpp_def.m_asset_name);

            kainjow::mustache::data type_check_data;
            type_check_data.set("type", cpp_def.m_asset_name);
            type_check_data.set("extension_var", extension_var);
            type_checks_data << type_check_data;

            kainjow::mustache::data extension_data;
            extension_data.set("extension_var", extension_var);
            extensions_data2 << extension_data;

            asset_num++;
        }
    }

    for (int i = 0; i < type_names.size(); i++) {
        auto& name = type_names[i];
        if (i != type_names.size() - 1) {
            names_data << kainjow::mustache::data{"name", name + ","};
        } else {
            names_data << kainjow::mustache::data{"name", name};
        }
    }

    data.set("includes", includes_data);
    data.set("asset_extensions", extensions_data);
    data.set("extensions", extensions_data2);
    data.set("asset_names", names_data);
    data.set("type_check", type_checks_data);
    data.set("asset_num", std::to_string(asset_num));

    auto& mustache = MustacheManager::GetInst().m_asset_info_header_mustache;
    return mustache.render(data);
}

std::string GenerateAssetInfoImplCode(const SchemaInfoManager& manager) {
    auto& mustache = MustacheManager::GetInst().m_asset_info_impl_mustache;
    kainjow::mustache::data data;
    kainjow::mustache::data type_datas{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        for (auto& clazz : info.m_classes) {
            if (!clazz.is_asset) {
                continue;
            }

            kainjow::mustache::data data;
            data.set("type", clazz.m_name);
            type_datas << data;
        }

        for (auto& cpp_def : info.m_cpp_asset_defs) {
            auto extension_var = cpp_def.m_asset_name +
                                 std::string{ClassInfo::ExtensionVarSuffix};

            kainjow::mustache::data data;
            data.set("type", cpp_def.m_asset_name);
            type_datas << data;
        }
    }

    data.set("load_methods", type_datas);

    return mustache.render(data);
}

std::string GenerateAssetSerializeTotleHeaderCode(
    const SchemaInfoManager& manager) {
    auto& mustache =
        MustacheManager::GetInst().m_asset_serialize_header_mustache;
    kainjow::mustache::data data;
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        includes_data << kainjow::mustache::data{
            "include", info.m_generate_filename.string()};
    }

    data.set("includes", includes_data);

    return mustache.render(data);
}

std::string GenerateAssetDisplayTotleHeaderCode(
    const SchemaInfoManager& manager) {
    auto& mustache = MustacheManager::GetInst().m_asset_display_header_mustache;
    kainjow::mustache::data data;
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        includes_data << kainjow::mustache::data{
            "include", info.m_generate_filename.string()};
    }

    data.set("includes", includes_data);

    return mustache.render(data);
}

std::string GenerateEnumScriptBindHeaderCode(const EnumInfo& enum_info) {
    auto& mustache =
        MustacheManager::GetInst().m_enum_script_bind_header_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);

    return mustache.render(data);
}

std::string GenerateEnumScriptBindImplCode(const EnumInfo& enum_info) {
    auto& mustache =
        MustacheManager::GetInst().m_enum_script_bind_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);
    data.set("enum_name", enum_info.m_name);
    data.set("enum_flags_name", enum_info.m_name + "Flags");

    kainjow::mustache::data enum_values_data{
        kainjow::mustache::data::type::list};
    for (auto& item : enum_info.m_items) {
        kainjow::mustache::data enum_value_data;
        enum_value_data.set("enum_name", enum_info.m_name);
        enum_value_data.set("enum_value", item.m_name);
        enum_value_data.set("enum_item", enum_info.m_name + "::" + item.m_name);
        enum_values_data << enum_value_data;
    }
    data.set("enum_values", enum_values_data);

    return mustache.render(data);
}

std::string GenerateEnumStackSpecializationCode(const EnumInfo& enum_info) {
    auto& mustache =
        MustacheManager::GetInst().m_enum_stack_specialization_mustache;
    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);
    std::string value_list;
    for (auto& item : enum_info.m_items) {
        if (!value_list.empty()) value_list += ", ";
        value_list += enum_info.m_name + "::" + item.m_name;
    }
    data.set("enum_value_list", value_list.empty() ? "" : ", " + value_list);
    return mustache.render(data);
}

std::string GenerateClassScriptBindHeaderCode(const ClassInfo& info) {
    auto& mustache =
        MustacheManager::GetInst().m_class_script_bind_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    return mustache.render(data);
}

std::string GenerateClassScriptBindImplCode(const ClassInfo& info) {
    auto& mustache =
        MustacheManager::GetInst().m_class_script_bind_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    if (info.is_asset) {
        data.set("is_asset", true);
    }

    kainjow::mustache::data property_entries{
        kainjow::mustache::data::type::list};
    for (auto& prop : info.m_properties) {
        std::string property_name_with_prefix = "m_" + prop.m_name;
        kainjow::mustache::data prop_data;
        prop_data.set("type", info.m_name);
        prop_data.set("property_name", property_name_with_prefix);
        prop_data.set("property_type", prop.m_type);
        if (prop.m_is_optional) {
            prop_data.set("is_optional", true);
            prop_data.set("optional_inner_type",
                          extractOptionalInnerType(prop.m_type));
        }
        if (prop.m_is_flags) {
            prop_data.set("is_flags", true);
        }
        property_entries << prop_data;
    }

    data.set("property_entries", property_entries);

    return mustache.render(data);
}

std::string GenerateSchemaScriptBindHeaderCode(const SchemaInfo& schema) {
    auto& header_mustache =
        MustacheManager::GetInst().m_script_bind_header_mustache;

    kainjow::mustache::data data;

    auto header_filename = schema.m_pure_filename;
    header_filename.replace_extension(".hpp");
    data.set("schema_include", "schema/" + header_filename.string());

    kainjow::mustache::data enum_stack_specializations_data{
        kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumStackSpecializationCode(enum_value);
        enum_stack_specializations_data
            << kainjow::mustache::data{"stack_spec", code};
    }
    data.set("enum_stack_specializations", enum_stack_specializations_data);

    kainjow::mustache::data registers_data{kainjow::mustache::data::type::list};

    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumScriptBindHeaderCode(enum_value);
        registers_data << kainjow::mustache::data{"register", code};
    }

    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassScriptBindHeaderCode(class_value);
        registers_data << kainjow::mustache::data{"register", code};
    }

    data.set("registers", registers_data);

    return header_mustache.render(data);
}

std::string GenerateSchemaScriptBindImplCode(const SchemaInfo& schema) {
    auto& impl_mustache =
        MustacheManager::GetInst().m_script_bind_impl_mustache;

    kainjow::mustache::data data;

    auto header_filename = schema.m_pure_filename;
    header_filename.replace_extension(".hpp");
    auto header_file = "schema/binding/" + header_filename.string();
    data.set("header_file", header_file);

    // Include the schema header file (generated in schema/ directory)
    auto schema_file = "schema/" + header_filename.string();
    data.set("schema_file", schema_file);

    kainjow::mustache::data registers_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data bindings_data{kainjow::mustache::data::type::list};

    for (auto& enum_value : schema.m_enums) {
        auto register_code = GenerateEnumScriptBindHeaderCode(enum_value);
        auto bind_code = GenerateEnumScriptBindImplCode(enum_value);
        registers_data << kainjow::mustache::data{"register", register_code};
        bindings_data << kainjow::mustache::data{"binding", bind_code};
    }

    for (auto& class_value : schema.m_classes) {
        auto register_code = GenerateClassScriptBindHeaderCode(class_value);
        auto bind_code = GenerateClassScriptBindImplCode(class_value);
        registers_data << kainjow::mustache::data{"register", register_code};
        bindings_data << kainjow::mustache::data{"binding", bind_code};
    }

    data.set("registers", registers_data);
    data.set("bindings", bindings_data);

    return impl_mustache.render(data);
}

std::string GenerateBindingHeaderCode(const SchemaInfoManager& manager) {
    auto& header_mustache =
        MustacheManager::GetInst().m_binding_header_mustache;

    kainjow::mustache::data data;
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        auto header_filename = info.m_pure_filename;
        header_filename.replace_extension(".hpp");
        auto binding_include = "schema/binding/" + header_filename.string();
        auto schema_include = "schema/" + header_filename.string();
        includes_data << kainjow::mustache::data{"include", binding_include};
        includes_data << kainjow::mustache::data{"include", schema_include};
    }

    data.set("includes", includes_data);

    return header_mustache.render(data);
}

std::string GenerateBindingImplCode(const SchemaInfoManager& manager) {
    auto& impl_mustache = MustacheManager::GetInst().m_binding_impl_mustache;

    kainjow::mustache::data data;

    kainjow::mustache::data bindings_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data asset_manager_getters_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data asset_manager_classes_data{
        kainjow::mustache::data::type::list};
    kainjow::mustache::data asset_filename_checks_data{
        kainjow::mustache::data::type::list};
    std::unordered_set<std::string> seen_bind_names;
    std::unordered_set<std::string> seen_generic_asset_types;
    std::unordered_set<std::string> seen_filename_check_types;
    static const std::unordered_set<std::string> specialized_asset_types = {
        "Image", "Tilemap", "Animation", "Level", "Font", "ScriptBinaryData",
    };

    for (auto& info : manager.m_infos) {
        for (auto& enum_value : info.m_enums) {
            std::string func_name =
                "bind" + enum_value.m_name + "TypeFromSchema";
            if (seen_bind_names.insert(func_name).second)
                bindings_data
                    << kainjow::mustache::data{"binding", func_name + "(L);"};
        }

        for (auto& class_value : info.m_classes) {
            std::string func_name =
                "bind" + class_value.m_name + "TypeFromSchema";
            if (seen_bind_names.insert(func_name).second)
                bindings_data
                    << kainjow::mustache::data{"binding", func_name + "(L);"};

            if (class_value.is_asset &&
                seen_filename_check_types.insert(class_value.m_name).second) {
                kainjow::mustache::data check_data;
                check_data.set("type", class_value.m_name);
                asset_filename_checks_data << check_data;
            }

            if (class_value.is_asset &&
                specialized_asset_types.count(class_value.m_name) == 0 &&
                seen_generic_asset_types.insert(class_value.m_name).second) {
                kainjow::mustache::data manager_data;
                manager_data.set("type", class_value.m_name);
                asset_manager_getters_data << manager_data;
                asset_manager_classes_data << manager_data;
            }
        }

        for (auto& cpp_def : info.m_cpp_asset_defs) {
            if (seen_filename_check_types.insert(cpp_def.m_asset_name).second) {
                kainjow::mustache::data check_data;
                check_data.set("type", cpp_def.m_asset_name);
                asset_filename_checks_data << check_data;
            }
        }
    }

    data.set("bindings", bindings_data);
    data.set("asset_manager_getters", asset_manager_getters_data);
    data.set("asset_manager_classes", asset_manager_classes_data);
    data.set("asset_filename_checks", asset_filename_checks_data);

    return impl_mustache.render(data);
}

bool IsLuauPrimitiveType(const std::string& name) {
    return name == "number" || name == "string" || name == "boolean";
}

std::string EnsureTLPrefixForSchema(
    const std::string& luau_type,
    const std::unordered_set<std::string>& schema_defined_type_names) {
    std::string out;
    out.reserve(luau_type.size() * 2);
    for (size_t i = 0; i < luau_type.size();) {
        if (luau_type[i] == ' ' || luau_type[i] == '{' || luau_type[i] == '}' ||
            luau_type[i] == ',' || luau_type[i] == '?' || luau_type[i] == ':') {
            out += luau_type[i++];
            continue;
        }
        if ((luau_type[i] >= 'a' && luau_type[i] <= 'z') ||
            (luau_type[i] >= 'A' && luau_type[i] <= 'Z') ||
            luau_type[i] == '_') {
            size_t start = i;
            while (i < luau_type.size() &&
                   (std::isalnum(static_cast<unsigned char>(luau_type[i])) ||
                    luau_type[i] == '_'))
                ++i;
            std::string id = luau_type.substr(start, i - start);
            const bool is_primitive = IsLuauPrimitiveType(id);
            const bool is_schema_type =
                (schema_defined_type_names.count(id) != 0);
            if (!is_primitive && !is_schema_type) out += "TL.";
            out += id;
            continue;
        }
        out += luau_type[i++];
    }
    return out;
}

bool IsLuaKeyword(const std::string& name) {
    static const std::unordered_set<std::string> keywords = {
        "and",      "break",  "do",   "else", "elseif", "end",   "false", "for",
        "function", "goto",   "if",   "in",   "local",  "nil",   "not",   "or",
        "repeat",   "return", "then", "true", "until",  "while",
    };
    return keywords.count(name) != 0;
}

std::string ConvertCppTypeToLuauType(const std::string& cpp_type) {
    static const std::unordered_map<std::string, std::string> Cpp2Luau = {
        {      "float",  "number"},
        {     "double",  "number"},
        {        "int",  "number"},
        {     "int8_t",  "number"},
        {    "int16_t",  "number"},
        {    "int32_t",  "number"},
        {    "int64_t",  "number"},
        {    "uint8_t",  "number"},
        {   "uint16_t",  "number"},
        {   "uint32_t",  "number"},
        {   "uint64_t",  "number"},
        {"std::string",  "string"},
        {       "bool", "boolean"},
    };
    auto it = Cpp2Luau.find(cpp_type);
    if (it != Cpp2Luau.end()) return it->second;
    std::string inner = extractOptionalInnerType(cpp_type);
    if (!inner.empty()) return ConvertCppTypeToLuauType(inner) + "?";
    std::string flags_inner = extractFlagsInnerType(cpp_type);
    if (!flags_inner.empty()) return flags_inner + "Flags";
    std::string vec_inner = extractVectorInnerType(cpp_type);
    if (!vec_inner.empty())
        return "{ " + ConvertCppTypeToLuauType(vec_inner) + " }";
    std::string arr_inner = extractArrayInnerType(cpp_type);
    if (!arr_inner.empty())
        return "{ " + ConvertCppTypeToLuauType(arr_inner) + " }";
    if (!extractUnorderedMapInnerType(cpp_type).empty()) return "{}";
    return cpp_type;
}

std::string CppToLuauForProto(const std::string& cpp_type) {
    if (cpp_type == "std::string" || cpp_type == "std::string_view")
        return "string";
    if (cpp_type == "bool") return "boolean";
    if (CppTypeToProtoType.find(cpp_type) != CppTypeToProtoType.end())
        return "number";
    return cpp_type;
}

std::string GenerateClassLuauType(
    const ClassInfo& info,
    const std::unordered_set<std::string>& schema_defined_type_names,
    bool use_tl_prefix) {
    std::string out = "export type " + info.m_name + " = {\n";
    for (const auto& p : info.m_properties) {
        std::string luau_type = ConvertCppTypeToLuauType(p.m_type);
        if (p.m_is_optional && luau_type.back() != '?') luau_type += "?";
        if (use_tl_prefix)
            luau_type =
                EnsureTLPrefixForSchema(luau_type, schema_defined_type_names);
        std::string key = IsLuaKeyword(p.m_name) ? ("[\"" + p.m_name + "\"]")
                                                 : "m_" + p.m_name;
        out += "\t" + key + ": " + luau_type + ",\n";
    }
    out += "} & (() -> " + info.m_name + ")\n";
    return out;
}

std::string GenerateEnumLuauType(const EnumInfo& info) {
    std::string out = "export type " + info.m_name + " = {\n";
    for (const auto& item : info.m_items) {
        std::string key = IsLuaKeyword(item.m_name)
                              ? ("[\"" + item.m_name + "\"]")
                              : item.m_name;
        out += "\t" + key + ": number,\n";
    }
    out += "}\n";
    return out;
}

std::string GenerateAssetHandleLuauType(const std::string& asset_class_name) {
    std::string handle_name = asset_class_name + "Handle";
    return "export type " + handle_name +
           " = { IsValid: (self: " + handle_name + ") -> boolean, " +
           "GetFilename: (self: " + handle_name + ") -> Path?, " +
           "GetUUID: (self: " + handle_name + ") -> UUID } & " +
           asset_class_name + "\n";
}

std::string GenerateAssetLoadSaveLuauTypes(
    const std::string& asset_class_name) {
    std::string out;
    out += "export type LoadAsset" + asset_class_name + " = (path: Path) -> " +
           asset_class_name + "\n";
    out += "export type SaveAsset" + asset_class_name +
           " = (handle: " + asset_class_name + "Handle, path: Path) -> ()\n";
    return out;
}

std::string GenerateAssetFilenameIsLuauType(
    const std::string& asset_class_name) {
    return "export type FilenameIs" + asset_class_name +
           " = (filename: Path) -> boolean\n";
}

std::string GenerateFlagsLuauType(const std::string& enum_class_name) {
    const std::string flags_name = enum_class_name + "Flags";
    std::string out;
    out += "export type " + flags_name + " = {\n";
    out += "\tValue: (self: " + flags_name + ") -> number,\n";
    out += "\tHas: (self: " + flags_name + ", value: " + enum_class_name +
           ") -> boolean,\n";
    out += "\tRemove: (self: " + flags_name + ", value: " + enum_class_name +
           ") -> (),\n";
    out += "\t__bor: (self: " + flags_name + ", value: " + enum_class_name +
           ") -> " + flags_name + ",\n";
    out += "\t__band: (self: " + flags_name + ", value: " + enum_class_name +
           ") -> " + flags_name + ",\n";
    out += "\t__bnot: (self: " + flags_name + ") -> " + flags_name + ",\n";
    out += "\t__tostring: (self: " + flags_name + ") -> string,\n";
    out += "} & (() -> " + flags_name + ") & ((" + enum_class_name + ") -> " +
           flags_name + ") & ((number) -> " + flags_name + ")\n";
    return out;
}

std::string GenerateGenericAssetManagerLuauType(
    const std::string& asset_class_name) {
    const std::string manager_name = asset_class_name + "AssetManager";
    const std::string handle_name = asset_class_name + "Handle";
    std::string out;
    out += "export type " + manager_name + " = {\n";
    out += "\tCreate: (self: " + manager_name + ") -> " + handle_name + ",\n";
    out += "\tLoad: (self: " + manager_name +
           ", path: string, force: boolean?) -> " + handle_name + ",\n";
    out += "\tFind: (self: " + manager_name + ", path: string) -> " +
           handle_name + ",\n";
    out += "\tUnload: (self: " + manager_name + ", handle: " + handle_name +
           ") -> (),\n";
    out += "\tReload: (self: " + manager_name + ", handle: " + handle_name +
           ") -> (),\n";
    out += "\tClear: (self: " + manager_name + ") -> (),\n";
    out += "}\n";
    return out;
}

std::string GenerateSchemaTypesLuauCode(const SchemaInfoManager& manager) {
    std::unordered_set<std::string> schema_defined_type_names;
    for (const auto& schema : manager.m_infos) {
        for (const auto& clazz : schema.m_classes) {
            schema_defined_type_names.insert(clazz.m_name);
            if (clazz.is_asset)
                schema_defined_type_names.insert(clazz.m_name + "Handle");
            for (const auto& prop : clazz.m_properties) {
                if (prop.m_is_flags) {
                    std::string flags_inner =
                        extractFlagsInnerType(prop.m_type);
                    if (!flags_inner.empty())
                        schema_defined_type_names.insert(flags_inner + "Flags");
                }
            }
        }
        for (const auto& enum_info : schema.m_enums)
            schema_defined_type_names.insert(enum_info.m_name);
    }

    auto& class_t = MustacheManager::GetInst().m_schema_class_luau_mustache;
    auto& enum_t = MustacheManager::GetInst().m_schema_enum_luau_mustache;
    auto& flags_t = MustacheManager::GetInst().m_schema_flags_luau_mustache;
    auto& asset_t = MustacheManager::GetInst().m_schema_asset_luau_mustache;
    auto& fnis_t =
        MustacheManager::GetInst().m_schema_filename_is_luau_mustache;
    auto& amgr_t =
        MustacheManager::GetInst().m_schema_asset_manager_luau_mustache;

    std::string out;
    std::unordered_set<std::string> emitted_classes;
    std::unordered_set<std::string> emitted_enums;
    std::unordered_set<std::string> emitted_handles;
    std::unordered_set<std::string> emitted_flags;
    std::unordered_set<std::string> emitted_generic_asset_managers;
    std::unordered_set<std::string> emitted_filename_is_types;
    std::vector<std::string> generic_asset_types;

    bool use_tl_prefix = false;
    for (const auto& schema : manager.m_infos) {
        for (const auto& clazz : schema.m_classes) {
            if (emitted_classes.insert(clazz.m_name).second) {
                kainjow::mustache::data cdata;
                cdata.set("class_name", clazz.m_name);
                kainjow::mustache::data props{
                    kainjow::mustache::data::type::list};
                for (const auto& p : clazz.m_properties) {
                    std::string luau_type =
                        ConvertCppTypeToLuauType(p.m_type);
                    if (p.m_is_optional && luau_type.back() != '?')
                        luau_type += "?";
                    if (use_tl_prefix)
                        luau_type = EnsureTLPrefixForSchema(
                            luau_type, schema_defined_type_names);
                    std::string key =
                        IsLuaKeyword(p.m_name)
                            ? ("[\"" + p.m_name + "\"]")
                            : "m_" + p.m_name;
                    kainjow::mustache::data pd;
                    pd.set("key", key);
                    pd.set("luau_type", luau_type);
                    props << pd;
                }
                cdata.set("properties", props);
                out += class_t.render(cdata) + "\n";
            }
            if (clazz.is_asset &&
                emitted_handles.insert(clazz.m_name + "Handle").second) {
                kainjow::mustache::data adata;
                adata.set("class_name", clazz.m_name);
                adata.set("handle_name", clazz.m_name + "Handle");
                out += asset_t.render(adata);
            }
            if (clazz.is_asset &&
                emitted_filename_is_types.insert(clazz.m_name).second) {
                out += fnis_t.render({"class_name", clazz.m_name});
            }
            if (clazz.is_asset &&
                emitted_generic_asset_managers.insert(clazz.m_name).second) {
                generic_asset_types.push_back(clazz.m_name);
                kainjow::mustache::data mdata;
                mdata.set("class_name", clazz.m_name);
                mdata.set("handle_name", clazz.m_name + "Handle");
                out += amgr_t.render(mdata);
            }
        }
        for (const auto& cpp_def : schema.m_cpp_asset_defs) {
            if (emitted_filename_is_types.insert(cpp_def.m_asset_name).second) {
                out += fnis_t.render({"class_name", cpp_def.m_asset_name});
            }
        }
        for (const auto& enum_info : schema.m_enums) {
            if (emitted_flags.insert(enum_info.m_name).second) {
                kainjow::mustache::data fdata;
                fdata.set("flags_name", enum_info.m_name + "Flags");
                fdata.set("enum_name", enum_info.m_name);
                out += flags_t.render(fdata);
            }
            if (emitted_enums.insert(enum_info.m_name).second) {
                kainjow::mustache::data edata;
                edata.set("enum_name", enum_info.m_name);
                kainjow::mustache::data items{
                    kainjow::mustache::data::type::list};
                for (const auto& item : enum_info.m_items) {
                    std::string key = IsLuaKeyword(item.m_name)
                                          ? ("[\"" + item.m_name + "\"]")
                                          : item.m_name;
                    items << kainjow::mustache::data{"key", key};
                }
                edata.set("items", items);
                out += enum_t.render(edata) + "\n";
            }
        }
    }

    out += "\n-- Schema values (enums, FilenameIs*, asset managers, "
           "LoadAsset*, etc.) are "
           "bound under TL_Schema.\n";
    return out;
}

std::string GenerateProtoTypesLuauCode(const SchemaInfoManager& mgr) {
    std::string out;
    auto& class_t = MustacheManager::GetInst().m_proto_class_luau_mustache;
    auto& net_msg_t = MustacheManager::GetInst().m_proto_net_msg_luau_mustache;
    auto& wrapper_t =
        MustacheManager::GetInst().m_proto_net_msg_wrapper_luau_mustache;

    for (auto& schema_info : mgr.m_infos) {
        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.ShouldGenProto()) {
                continue;
            }

            kainjow::mustache::data cdata;
            cdata.set("class_name", class_info.m_name);
            cdata.set("has_schema", true);

            kainjow::mustache::data fields_list{
                kainjow::mustache::data::type::list};
            for (auto& property : class_info.m_properties) {
                if (!property.m_proto_id) {
                    continue;
                }

                kainjow::mustache::data fdata;
                fdata.set("field", "m_" + property.m_name);
                fdata.set("luau_type", CppToLuauForProto(property.m_type));

                bool is_message =
                    CppTypeToProtoType.find(property.m_type) ==
                    CppTypeToProtoType.end();
                if (is_message) {
                    fdata.set("has_has", true);
                }
                fields_list << fdata;
            }
            cdata.set("fields", fields_list);

            out += class_t.render(cdata) + "\n";
        }
    }

    // NetMsg oneof type
    {
        kainjow::mustache::data ndata;
        kainjow::mustache::data nfields{kainjow::mustache::data::type::list};
        for (auto& schema_info : mgr.m_infos) {
            for (auto& class_info : schema_info.m_classes) {
                if (!class_info.m_proto_id) {
                    continue;
                }
                kainjow::mustache::data fdata;
                fdata.set("field",
                          "m_" + toSnakeCase(class_info.m_name));
                fdata.set("type", class_info.m_name);
                fdata.set("has_has", true);
                nfields << fdata;
            }
            for (auto& enum_info : schema_info.m_enums) {
                if (!enum_info.m_proto_id) {
                    continue;
                }
                kainjow::mustache::data fdata;
                fdata.set("field",
                          "m_" + toSnakeCase(enum_info.m_name));
                fdata.set("type", enum_info.m_name);
                fdata.set("has_has", true);
                nfields << fdata;
            }
        }
        ndata.set("fields", nfields);
        out += net_msg_t.render(ndata) + "\n";
    }

    // NetMsg<proto::T> wrapper types
    for (auto& schema_info : mgr.m_infos) {
        for (auto& class_info : schema_info.m_classes) {
            if (!class_info.m_proto_id) {
                continue;
            }
            kainjow::mustache::data wdata;
            wdata.set("wrapper_name", "NetMsg_" + class_info.m_name);
            wdata.set("type", class_info.m_name);
            out += wrapper_t.render(wdata) + "\n";
        }
        for (auto& enum_info : schema_info.m_enums) {
            if (!enum_info.m_proto_id) {
                continue;
            }
            kainjow::mustache::data wdata;
            wdata.set("wrapper_name", "NetMsg_" + enum_info.m_name);
            wdata.set("type", enum_info.m_name);
            out += wrapper_t.render(wdata) + "\n";
        }
    }

    return out;
}

std::string GenerateCppAssetExtensionHeaderCode(
    const SchemaInfoManager& manager) {
    kainjow::mustache::data extensions{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        for (auto& def : info.m_cpp_asset_defs) {
            kainjow::mustache::data def_data;
            def_data.set("name", def.m_asset_name);
            def_data.set("extension", def.m_extension);

            extensions << def_data;
        }
    }

    kainjow::mustache::data extension_data;
    extension_data.set("extensions", extensions);
    std::string extension_content =
        MustacheManager::GetInst().m_cpp_asset_def_mustache.render(
            extension_data);

    kainjow::mustache::data header_data;
    header_data.set("content", extension_content);

    return MustacheManager::GetInst().m_cpp_asset_def_header_mustache.render(
        header_data);
}
