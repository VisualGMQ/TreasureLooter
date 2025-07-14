#include "code_generate.hpp"
#include "common.hpp"

std::string GenerateClassCode(const ClassInfo& info) {
    kainjow::mustache::data prop_datas{kainjow::mustache::data::type::list};

    auto& prop_mustache = MustacheManager::GetInst().m_property_mustache;
    for (auto& property : info.m_properties) {
        kainjow::mustache::data prop_data;
        prop_data.set("type", property.m_type);
        prop_data.set("name", property.m_name);

        auto prop_code = prop_mustache.render(prop_data);
        prop_datas << kainjow::mustache::data{"property", prop_code};
    }

    kainjow::mustache::data class_data;
    class_data.set("class_name", info.m_name);
    class_data.set("properties", prop_datas);
    auto& class_mustache = MustacheManager::GetInst().m_class_mustache;
    return class_mustache.render(class_data);
}

std::string GenerateSchemaCode(const SchemaInfo& schema_info) {
    auto& schema_mustache = MustacheManager::GetInst().m_schema_mustache;
    auto& include_mustache = MustacheManager::GetInst().m_include_mustache;

    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data include_datas{kainjow::mustache::data::type::list};
    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};

    for (auto& include : schema_info.m_includes) {
        include_datas << kainjow::mustache::data{
            "include",
            include_mustache.render({"filename", "\"" + include + "\""})};
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
    return schema_mustache.render(final_datas);
}

std::string GenerateEnumCode(const EnumInfo& enum_info) {
    auto& enum_mustache = MustacheManager::GetInst().m_enum_mustache;

    kainjow::mustache::data item_datas{kainjow::mustache::data::type::list};

    for (auto& item : enum_info.m_items) {
        std::string item_declare = item.m_name;
        if (item.m_value) {
            item_declare += " = " + std::to_string(item.m_value.value());
        }

        item_datas << kainjow::mustache::data{"item", item_declare};
    }

    kainjow::mustache::data enum_data;
    enum_data.set("items", item_datas);
    enum_data.set("name", enum_info.m_name);

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
    include_datas << kainjow::mustache::data{"include", final_path.c_str()};
    data.set("includes", include_datas);

    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumSerializeHeaderCode(enum_value);
        enum_datas << kainjow::mustache::data{"enum", code};
    }
    
    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassSerializeHeaderCode(class_value);
        class_datas << kainjow::mustache::data{"class", code};
    }

    data.set("enums", enum_datas);
    data.set("classes", class_datas);

    return header_mustache.render(data);
}

std::string GenerateSchemaSerializeImplCode(const SchemaInfo& schema) {
    auto& mustache = MustacheManager::GetInst().m_schema_serd_impl_mustache;

    kainjow::mustache::data data;
    data.set("filename", schema.m_pure_filename.string().c_str());

    kainjow::mustache::data enum_datas{kainjow::mustache::data::type::list};
    for (auto& enum_value : schema.m_enums) {
        auto code = GenerateEnumSerializeImplCode(enum_value);
        enum_datas << kainjow::mustache::data{"enum_impl", code};
    }
    
    kainjow::mustache::data class_datas{kainjow::mustache::data::type::list};
    for (auto& class_value : schema.m_classes) {
        auto code = GenerateClassSerializeImplCode(class_value);
        class_datas << kainjow::mustache::data{"class_impl", code};
    }

    data.set("enum_impls", enum_datas);
    data.set("class_impls", class_datas);

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

    return mustache.render(data);
}

std::string GenerateClassSerializeImplCode(const ClassInfo& info) {
    auto& mustache = MustacheManager::GetInst().m_class_serd_impl_mustache;

    kainjow::mustache::data data;
    data.set("type", info.m_name);

    kainjow::mustache::data properties_data{
        kainjow::mustache::data::type::list};
    for (auto& prop : info.m_properties) {
        properties_data << kainjow::mustache::data{"property", prop.m_name};
    }

    data.set("properties", properties_data);

    return mustache.render(data);
}
