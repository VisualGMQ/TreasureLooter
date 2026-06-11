#include "common.hpp"

#include "mustache.hpp"

#include <fstream>
#include <iostream>

MustacheManager& MustacheManager::GetInst() {
    static MustacheManager instance;
    return instance;
}

MustacheManager::MustacheManager()
    : m_class_mustache{
          readMustache("schema/schema_parser/mustaches/schema/class.mustache")},
      m_property_mustache{
          readMustache("schema/schema_parser/mustaches/schema/property.mustache")},
      m_schema_mustache{
          readMustache("schema/schema_parser/mustaches/schema/schema.mustache")},
      m_include_mustache{
          readMustache("schema/schema_parser/mustaches/schema/include.mustache")},
      m_enum_mustache{
          readMustache("schema/schema_parser/mustaches/schema/enum.mustache")},
      m_enum_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/enum_serd_header.mustache")},
      m_enum_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/enum_serd_impl.mustache")},
      m_schema_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/schema_serd_header.mustache")},
      m_schema_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/schema_serd_impl.mustache")},
      m_class_serd_header_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/class_serd_header.mustache")},
      m_class_serd_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/class_serd_impl.mustache")},
      m_asset_sl_header_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/asset_sl_header.mustache")},
      m_asset_sl_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/asset_sl_impl.mustache")},
      m_instance_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/display/instance_display_header.mustache")},
      m_instance_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/display/instance_display_impl.mustache")},
      m_enum_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/display/enum_display_impl.mustache")},
      m_enum_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/display/enum_display_header.mustache")},
      m_class_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/display/class_display_header.mustache")},
      m_class_display_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/display/class_display_impl.mustache")},
      m_proto_mustache{
          readMustache("schema/schema_parser/mustaches/proto/proto.mustache")},
      m_proto_class_declare_mustache{readMustache(
          "schema/schema_parser/mustaches/proto/proto_class_declare.mustache")},
      m_proto_enum_declare_mustache{readMustache(
          "schema/schema_parser/mustaches/proto/proto_enum_declare.mustache")},
      m_all_proto_mustache{
          readMustache("schema/schema_parser/mustaches/proto/all_proto.mustache")},
      m_net_msg_dispatch_impl_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_net_msg_dispatch_impl.mustache")},
      m_net_msg_dispatch_header_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_net_msg_dispatch_header.mustache")},
      m_proto_binding_header_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_binding_header.mustache")},
      m_proto_binding_impl_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_binding_impl.mustache")},
      m_proto_event_binding_header_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_event_binding_header.mustache")},
      m_proto_event_binding_impl_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_event_binding_impl.mustache")},
      m_proto_convert_header_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_convert_header.mustache")},
      m_proto_convert_impl_mustache{
          readMustache("schema/schema_parser/mustaches/proto/"
                       "proto_convert_impl.mustache")},
      m_asset_info_header_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/asset_info_header.mustache")},
      m_asset_info_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/asset_info_impl.mustache")},
      m_asset_serialize_header_mustache{readMustache(
          "schema/schema_parser/mustaches/serd/serialize_header.mustache")},
      m_asset_display_header_mustache{readMustache(
          "schema/schema_parser/mustaches/display/display_header.mustache")},
      m_enum_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/enum_script_bind_header.mustache")},
      m_enum_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/enum_script_bind_impl.mustache")},
      m_enum_stack_specialization_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/enum_stack_specialization.mustache")},
      m_class_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/class_script_bind_header.mustache")},
      m_class_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/class_script_bind_impl.mustache")},
      m_script_bind_header_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/script_bind_header.mustache")},
      m_script_bind_impl_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/script_bind_impl.mustache")},
      m_binding_header_mustache{readMustache(
          "schema/schema_parser/mustaches/binding/binding_header.mustache")},
      m_binding_impl_mustache{
          readMustache("schema/schema_parser/mustaches/binding/binding_impl.mustache")},
      m_cpp_asset_def_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/cpp_asset_def.mustache")},
      m_cpp_asset_def_header_mustache{readMustache(
          "schema/schema_parser/mustaches/asset/cpp_asset_def_header.mustache")},
      m_proto_class_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/proto_class_luau.mustache")},
      m_proto_net_msg_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/proto_net_msg_luau.mustache")},
      m_proto_net_msg_wrapper_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/proto_net_msg_wrapper_luau.mustache")},
      m_schema_class_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_class_luau.mustache")},
      m_schema_enum_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_enum_luau.mustache")},
      m_schema_flags_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_flags_luau.mustache")},
      m_schema_asset_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_asset_luau.mustache")},
      m_schema_filename_is_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_filename_is_luau.mustache")},
      m_schema_asset_manager_luau_mustache{readMustache(
          "schema/schema_parser/mustaches/schema_hint/schema_asset_manager_luau.mustache")} {}

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
