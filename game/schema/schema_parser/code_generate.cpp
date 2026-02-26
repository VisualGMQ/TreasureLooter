#include "code_generate.hpp"
#include "common.hpp"
#include "mustache.hpp"
#include <cctype>
#include <unordered_map>
#include <unordered_set>

// Forward declarations for property handling (defined later in this file)
static std::string ExtractOptionalInnerType(const std::string& cpp_type);
static std::string ExtractFlagsInnerType(const std::string& cpp_type);
static std::string ExtractVectorInnerType(const std::string& cpp_type);
static std::string ExtractHandleInnerType(const std::string& cpp_type);

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
        // optional/flags/array/handle: no get/set in struct; script binding uses lambda on member (m_xxx)
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
            include_mustache.render({"filename",
                                     "\"engine/" + include + "\""})};
    }

    if (schema_info.m_include_hints & IncludeHint::Option) {
        include_datas << kainjow::mustache::data{
            "include", include_mustache.render({"filename", "<optional>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::Flags) {
        include_datas << kainjow::mustache::data{
            "include", include_mustache.render({"filename", "\"engine/script/script_flags_binding.hpp\""})};
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
            "include",
            include_mustache.render({"filename", "<cstdint>"})};
    }
    if (schema_info.m_include_hints & IncludeHint::Handle) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "\"engine/handle.hpp\""})};
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "\"engine/script/script_handle_binding.hpp\""})};
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
        include_datas << kainjow::mustache::data
            {"include", "engine/" + include};
    }

    auto generate_header_filename = schema.m_pure_filename;
    generate_header_filename =
        generate_header_filename.replace_extension(".hpp");
    auto final_path = "schema/" + generate_header_filename.string();
    if (schema.m_include_hints & IncludeHint::Asset) {
        include_datas << kainjow::mustache::data{"include", "engine/asset.hpp"};
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
                                                "engine/asset.hpp"};
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
        property_data.set("is_optional", prop.m_optional ? "true" : "false");
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

    if (info.is_asset) {
        data.set("has_handle", true);
    }

    data.set("properties", properties_data);

    return mustache.render(data);
}

std::string GenerateSchemaDisplayHeaderCode(const SchemaInfo& schema) {
    auto& header_mustache =
        MustacheManager::GetInst().m_instance_display_header_mustache;

    kainjow::mustache::data data;
    kainjow::mustache::data include_datas{kainjow::mustache::data::type::list};

    for (auto& include : schema.m_includes) {
        include_datas << kainjow::mustache::data
            {"include", "engine/" + include};
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

std::string GenerateAssetInfoHeaderCode(const SchemaInfoManager& manager) {
    kainjow::mustache::data data;
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data extensions_data
        {kainjow::mustache::data::type::list};
    kainjow::mustache::data names_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data type_checks_data{
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

            kainjow::mustache::data type_check_data;
            type_check_data.set("type", clazz.m_name);
            type_check_data.set("extension_var", clazz.m_asset_extension_var);
            type_checks_data << type_check_data;
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
    data.set("asset_names", names_data);
    data.set("type_check", type_checks_data);
    data.set("asset_num", std::to_string(asset_num));

    auto& mustache = MustacheManager::GetInst().m_asset_info_header_mustache;
    return mustache.render(data);
}

std::string GenerateAssetInfoImplCode(const SchemaInfoManager& manager) {
    auto& mustache = MustacheManager::GetInst().m_asset_info_impl_mustache;
    kainjow::mustache::data data;
    kainjow::mustache::data type_check_data
        {kainjow::mustache::data::type::list};
    kainjow::mustache::data includes_data{kainjow::mustache::data::type::list};

    for (auto& info : manager.m_infos) {
        bool included = false;
        for (auto& clazz : info.m_classes) {
            if (!clazz.is_asset) {
                continue;
            }

            if (!included) {
                includes_data << kainjow::mustache::data{
                    "include", info.m_generate_filename.string()};
                included = true;
            }

            kainjow::mustache::data extension_data;
            extension_data.set("type", clazz.m_name);
            extension_data.set("extension_var", clazz.m_asset_extension_var);
            type_check_data << extension_data;
        }
    }

    data.set("type_check", type_check_data);
    data.set("includes", includes_data);

    return mustache.render(data);
}

std::string GenerateAssetSerializeTotleHeaderCode(
    const SchemaInfoManager& manager) {
    auto& mustache = MustacheManager::GetInst().
        m_asset_serialize_header_mustache;
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
    auto& mustache = MustacheManager::GetInst().m_enum_script_bind_header_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);

    return mustache.render(data);
}

std::string GenerateEnumScriptBindImplCode(const EnumInfo& enum_info) {
    auto& mustache = MustacheManager::GetInst().m_enum_script_bind_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", enum_info.m_name);
    data.set("enum_name", enum_info.m_name);

    kainjow::mustache::data enum_values_data{kainjow::mustache::data::type::list};
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
    auto& mustache = MustacheManager::GetInst().m_enum_stack_specialization_mustache;
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
    auto& mustache = MustacheManager::GetInst().m_class_script_bind_header_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    return mustache.render(data);
}

std::string GenerateClassScriptBindImplCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_script_bind_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    if (info.is_asset) {
        data.set("is_asset", true);
    }

    kainjow::mustache::data property_entries{kainjow::mustache::data::type::list};

    for (auto& prop : info.m_properties) {
        std::string property_name_with_prefix = "m_" + prop.m_name;
        kainjow::mustache::data prop_data;
        prop_data.set("type", info.m_name);
        prop_data.set("property_name", property_name_with_prefix);
        property_entries << prop_data;
    }

    data.set("property_entries", property_entries);

    return mustache.render(data);
}

std::string GenerateSchemaScriptBindHeaderCode(const SchemaInfo& schema) {
    auto& header_mustache = MustacheManager::GetInst().m_script_bind_header_mustache;

    kainjow::mustache::data data;

    auto header_filename = schema.m_pure_filename;
    header_filename.replace_extension(".hpp");
    data.set("schema_include", "schema/" + header_filename.string());

    kainjow::mustache::data enum_stack_specializations_data{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumStackSpecializationCode(enum_value);
        enum_stack_specializations_data << kainjow::mustache::data{"stack_spec", code};
    }
    data.set("enum_stack_specializations", enum_stack_specializations_data);

    kainjow::mustache::data registers_data{kainjow::mustache::data::type::list};
    kainjow::mustache::data bindings_data{kainjow::mustache::data::type::list};

    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumScriptBindHeaderCode(enum_value);
        registers_data << kainjow::mustache::data{"register", code};
        bindings_data << kainjow::mustache::data{"binding", code};
    }

    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassScriptBindHeaderCode(class_value);
        registers_data << kainjow::mustache::data{"register", code};
        bindings_data << kainjow::mustache::data{"binding", code};
    }

    data.set("registers", registers_data);
    data.set("bindings", bindings_data);

    return header_mustache.render(data);
}

std::string GenerateSchemaScriptBindImplCode(const SchemaInfo& schema) {
    auto& impl_mustache = MustacheManager::GetInst().m_script_bind_impl_mustache;

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
    auto& header_mustache = MustacheManager::GetInst().m_binding_header_mustache;

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
    std::unordered_set<std::string> seen_bind_names;

    for (auto& info : manager.m_infos) {
        for (auto& enum_value : info.m_enums) {
            std::string func_name = "bind" + enum_value.m_name + "TypeFromSchema";
            if (seen_bind_names.insert(func_name).second)
                bindings_data << kainjow::mustache::data{"binding", func_name + "(L);"};
        }

        for (auto& class_value : info.m_classes) {
            std::string func_name = "bind" + class_value.m_name + "TypeFromSchema";
            if (seen_bind_names.insert(func_name).second)
                bindings_data << kainjow::mustache::data{"binding", func_name + "(L);"};
        }
    }

    data.set("bindings", bindings_data);

    return impl_mustache.render(data);
}

static const std::unordered_map<std::string, std::string> Cpp2AngelScriptTypeMap = {
    // Integer types
    {"uint8_t", "uint8"},
    {"uint16_t", "uint16"},
    {"uint32_t", "uint32"},
    {"uint64_t", "uint64"},
    {"int8_t", "int8"},
    {"int16_t", "int16"},
    {"int32_t", "int32"},
    {"int64_t", "int64"},
    
    // Standard types
    {"std::string", "string"},
};

static std::string ExtractOptionalInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::optional<";
    if (cpp_type.size() <= prefix.size() + 1 || cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<') ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0) return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

static std::string ExtractFlagsInnerType(const std::string& cpp_type) {
    const std::string prefix = "Flags<";
    if (cpp_type.size() <= prefix.size() + 1 || cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<') ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0) return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

std::string ConvertCppTypeToAngelScript(const std::string& cpp_type) {
    auto it = Cpp2AngelScriptTypeMap.find(cpp_type);
    if (it != Cpp2AngelScriptTypeMap.end()) {
        return it->second;
    }
    std::string inner = ExtractOptionalInnerType(cpp_type);
    if (!inner.empty()) {
        return "Optional<" + ConvertCppTypeToAngelScript(inner) + ">";
    }
    std::string flags_inner = ExtractFlagsInnerType(cpp_type);
    if (!flags_inner.empty()) {
        return "Flags<" + ConvertCppTypeToAngelScript(flags_inner) + ">";
    }
    std::string vector_inner = ExtractVectorInnerType(cpp_type);
    if (!vector_inner.empty()) {
        return "array<" + ConvertCppTypeToAngelScript(vector_inner) + ">";
    }
    std::string handle_inner = ExtractHandleInnerType(cpp_type);
    if (!handle_inner.empty()) {
        return "Handle<" + ConvertCppTypeToAngelScript(handle_inner) + ">";
    }
    return cpp_type;
}

static std::string ExtractHandleInnerType(const std::string& cpp_type) {
    const std::string suffix = "Handle";
    if (cpp_type.size() > suffix.size() &&
        cpp_type.compare(cpp_type.size() - suffix.size(), suffix.size(), suffix) == 0)
        return cpp_type.substr(0, cpp_type.size() - suffix.size());
    return {};
}

static std::string ExtractVectorInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::vector<";
    if (cpp_type.size() <= prefix.size() + 1 || cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    int depth = 1;
    for (size_t i = prefix.size(); i < cpp_type.size() - 1; ++i) {
        char c = cpp_type[i];
        if (c == '<') ++depth;
        else if (c == '>') {
            --depth;
            if (depth == 0) return cpp_type.substr(prefix.size(), i - prefix.size());
        }
    }
    return cpp_type.substr(prefix.size(), cpp_type.size() - prefix.size() - 1);
}

static std::string ExtractArrayInnerType(const std::string& cpp_type) {
    const std::string prefix = "std::array<";
    if (cpp_type.size() <= prefix.size() + 1 || cpp_type.substr(0, prefix.size()) != prefix || cpp_type.back() != '>')
        return {};
    size_t comma = cpp_type.find(',', prefix.size());
    if (comma == std::string::npos) return {};
    return cpp_type.substr(prefix.size(), comma - prefix.size());
}

static bool IsLuauPrimitiveType(const std::string& name) {
    return name == "number" || name == "string" || name == "boolean";
}

static std::string EnsureTLPrefixForSchema(
    const std::string& luau_type,
    const std::unordered_set<std::string>& schema_defined_type_names) {
    std::string out;
    out.reserve(luau_type.size() * 2);
    for (size_t i = 0; i < luau_type.size(); ) {
        if (luau_type[i] == ' ' || luau_type[i] == '{' || luau_type[i] == '}' || luau_type[i] == ',' ||
            luau_type[i] == '?' || luau_type[i] == ':') {
            out += luau_type[i++];
            continue;
        }
        if ((luau_type[i] >= 'a' && luau_type[i] <= 'z') || (luau_type[i] >= 'A' && luau_type[i] <= 'Z') || luau_type[i] == '_') {
            size_t start = i;
            while (i < luau_type.size() && (std::isalnum(static_cast<unsigned char>(luau_type[i])) || luau_type[i] == '_'))
                ++i;
            std::string id = luau_type.substr(start, i - start);
            const bool is_primitive = IsLuauPrimitiveType(id);
            const bool is_schema_type = (schema_defined_type_names.count(id) != 0);
            if (!is_primitive && !is_schema_type)
                out += "TL.";
            out += id;
            continue;
        }
        out += luau_type[i++];
    }
    return out;
}

static bool IsLuaKeyword(const std::string& name) {
    static const std::unordered_set<std::string> keywords = {
        "and", "break", "do", "else", "elseif", "end", "false", "for", "function",
        "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return",
        "then", "true", "until", "while",
    };
    return keywords.count(name) != 0;
}

static std::string ConvertCppTypeToLuauType(const std::string& cpp_type) {
    static const std::unordered_map<std::string, std::string> Cpp2Luau = {
        {"float", "number"},
        {"double", "number"},
        {"int", "number"},
        {"int8_t", "number"},
        {"int16_t", "number"},
        {"int32_t", "number"},
        {"int64_t", "number"},
        {"uint8_t", "number"},
        {"uint16_t", "number"},
        {"uint32_t", "number"},
        {"uint64_t", "number"},
        {"std::string", "string"},
        {"bool", "boolean"},
    };
    auto it = Cpp2Luau.find(cpp_type);
    if (it != Cpp2Luau.end())
        return it->second;
    std::string inner = ExtractOptionalInnerType(cpp_type);
    if (!inner.empty())
        return ConvertCppTypeToLuauType(inner) + "?";
    if (!ExtractFlagsInnerType(cpp_type).empty())
        return "number";
    std::string vec_inner = ExtractVectorInnerType(cpp_type);
    if (!vec_inner.empty())
        return "{ " + ConvertCppTypeToLuauType(vec_inner) + " }";
    std::string arr_inner = ExtractArrayInnerType(cpp_type);
    if (!arr_inner.empty())
        return "{ " + ConvertCppTypeToLuauType(arr_inner) + " }";
    return cpp_type;
}

static std::string GenerateClassLuauType(
    const ClassInfo& info,
    const std::unordered_set<std::string>& schema_defined_type_names,
    bool use_tl_prefix) {
    std::string out = "export type " + info.m_name + " = {\n";
    for (const auto& p : info.m_properties) {
        std::string luau_type = ConvertCppTypeToLuauType(p.m_type);
        if (p.m_optional && luau_type.back() != '?')
            luau_type += "?";
        if (use_tl_prefix)
            luau_type = EnsureTLPrefixForSchema(luau_type, schema_defined_type_names);
        std::string key = IsLuaKeyword(p.m_name) ? ("[\"" + p.m_name + "\"]") : p.m_name;
        out += "\t" + key + ": " + luau_type + ",\n";
    }
    out += "}\n";
    return out;
}

static std::string GenerateEnumLuauType(const EnumInfo& info) {
    std::string out = "export type " + info.m_name + " = {\n";
    for (const auto& item : info.m_items) {
        std::string key = IsLuaKeyword(item.m_name) ? ("[\"" + item.m_name + "\"]") : item.m_name;
        out += "\t" + key + ": number,\n";
    }
    out += "}\n";
    return out;
}

static std::string GenerateAssetHandleLuauType(const std::string& asset_class_name) {
    std::string handle_name = asset_class_name + "Handle";
    return "export type " + handle_name + " = { IsValid: (self: " + handle_name + ") -> boolean }\n";
}

std::string GenerateSchemaTypesLuauCode(const SchemaInfoManager& manager) {
    std::unordered_set<std::string> schema_defined_type_names;
    for (const auto& schema : manager.m_infos) {
        for (const auto& clazz : schema.m_classes) {
            schema_defined_type_names.insert(clazz.m_name);
            if (clazz.is_asset)
                schema_defined_type_names.insert(clazz.m_name + "Handle");
        }
        for (const auto& enum_info : schema.m_enums)
            schema_defined_type_names.insert(enum_info.m_name);
    }

    std::string out = "------------------------------------------------------\n"
                      "-- CODE GENERATED BY SCHEMA GENERATOR, DO NOT EDIT! --\n"
                      "------------------------------------------------------\n"
                      "\n";
    std::unordered_set<std::string> emitted_classes;
    std::unordered_set<std::string> emitted_enums;
    std::unordered_set<std::string> emitted_handles;
    bool use_tl_prefix = false;
    for (const auto& schema : manager.m_infos) {
        for (const auto& clazz : schema.m_classes) {
            if (emitted_classes.insert(clazz.m_name).second)
                out += GenerateClassLuauType(clazz, schema_defined_type_names, use_tl_prefix) + "\n";
            if (clazz.is_asset && emitted_handles.insert(clazz.m_name + "Handle").second)
                out += GenerateAssetHandleLuauType(clazz.m_name) + "\n";
        }
        for (const auto& enum_info : schema.m_enums) {
            if (emitted_enums.insert(enum_info.m_name).second)
                out += GenerateEnumLuauType(enum_info) + "\n";
        }
    }

    out += "return TL";
    return out;
}