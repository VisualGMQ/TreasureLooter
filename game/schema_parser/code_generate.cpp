#include "code_generate.hpp"
#include "common.hpp"

std::string generateClassCode(const ClassInfo& info) {
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

std::string generateSchemaCode(const SchemaInfo& schema_info) {
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
        std::string code = generateClassCode(clazz);
        class_datas << kainjow::mustache::data{"class", code};
    }
    
    for (auto& enum_info : schema_info.m_enums) {
        std::string code = generateEnumCode(enum_info);
        enum_datas << kainjow::mustache::data{"enum", code};
    }

    kainjow::mustache::data final_datas;
    final_datas.set("classes", class_datas);
    final_datas.set("includes", include_datas);
    final_datas.set("enums", enum_datas);
    return schema_mustache.render(final_datas);
}

std::string generateEnumCode(const EnumInfo& enum_info) {
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