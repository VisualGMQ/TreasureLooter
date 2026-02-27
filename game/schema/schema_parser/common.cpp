#include "common.hpp"

#include "mustache.hpp"

#include <fstream>
#include <iostream>

MustacheManager& MustacheManager::GetInst() {
    static MustacheManager instance;
    return instance;
}

MustacheManager::MustacheManager()
    : m_class_mustache{readMustache(
          "schema/schema_parser/mustaches/class.mustache")},
      m_property_mustache{
          readMustache("schema/schema_parser/mustaches/property.mustache")},
      m_schema_mustache{
          readMustache("schema/schema_parser/mustaches/schema.mustache")},
      m_include_mustache{
          readMustache("schema/schema_parser/mustaches/include.mustache")},
      m_enum_mustache{
          readMustache("schema/schema_parser/mustaches/enum.mustache")},
      m_enum_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_serd_header.mustache")},
      m_enum_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_serd_impl.mustache")},
      m_schema_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_serd_header.mustache")},
      m_schema_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_serd_impl.mustache")},
      m_class_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/class_serd_header.mustache")},
      m_class_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/class_serd_impl.mustache")},
      m_asset_sl_header_mustache{readMustache(
          "schema/schema_parser/mustaches/asset_sl_header.mustache")},
      m_asset_sl_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/asset_sl_impl.mustache")},
      m_instance_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/instance_display_header.mustache")},
      m_instance_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/instance_display_impl.mustache")},
      m_enum_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_display_impl.mustache")},
      m_enum_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_display_header.mustache")},
      m_class_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/class_display_header.mustache")},
      m_class_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/class_display_impl.mustache")},
      m_asset_info_header_mustache{readMustache(
          "schema/schema_parser/mustaches/asset_info_header.mustache")},
      m_asset_info_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/asset_info_impl.mustache")},
      m_asset_serialize_header_mustache{readMustache(
          "schema/schema_parser/mustaches/serialize_header.mustache")},
      m_asset_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/display_header.mustache")},
      m_enum_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_script_bind_header.mustache")},
      m_enum_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_script_bind_impl.mustache")},
      m_enum_stack_specialization_mustache{readMustache(
          "schema/schema_parser/mustaches/enum_stack_specialization.mustache")},
      m_class_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/class_script_bind_header.mustache")},
      m_class_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/class_script_bind_impl.mustache")},
      m_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/script_bind_header.mustache")},
      m_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/script_bind_impl.mustache")},
      m_binding_header_mustache{readMustache(
          "schema/schema_parser/mustaches/binding_header.mustache")},
      m_binding_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/binding_impl.mustache")},
      m_cpp_asset_def_mustache{readMustache(
          "schema/schema_parser/mustaches/cpp_asset_def.mustache")},
      m_cpp_asset_def_header_mustache{readMustache(
          "schema/schema_parser/mustaches/cpp_asset_def_header.mustache")} {}

kainjow::mustache::mustache MustacheManager::readMustache(
    const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Can't find mustache file " << path << std::endl;
        assert(false);
    }

    auto content = std::string(std::istreambuf_iterator<char>(file), {});
    return kainjow::mustache::mustache{content};
}

std::string GetSchemaFileGenerateHeaderFilepath(const std::string& path) {
    return "schema/" + path + ".hpp";
}

std::string GetDisplayFileGenerateHeaderFilepath(const std::string& path) {
    return "schema/display/" + path + ".hpp";
}

std::string GetSerdFileGenerateHeaderFilepath(const std::string& path) {
    return "schema/serialize/" + path + ".hpp";
}

std::string GetSchemaFileGenerateImplFilepath(const std::string& path) {
    return "schema/" + path + ".cpp";
}

std::string GetDisplayFileGenerateImplFilepath(const std::string& path) {
    return "schema/display/" + path + ".cpp";
}

std::string GetSerdFileGenerateImplFilepath(const std::string path) {
    return "schema/serialize/" + path + ".cpp";
}
