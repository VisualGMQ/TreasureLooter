#include "common.hpp"
#include <fstream>
#include <iostream>

MustacheManager& MustacheManager::GetInst() {
    static MustacheManager instance;
    return instance;
}

MustacheManager::MustacheManager()
    : m_class_mustache{readMustache("schema_parser/mustaches/class.mustache")},
      m_property_mustache{
          readMustache("schema_parser/mustaches/property.mustache")},
      m_schema_mustache{
          readMustache("schema_parser/mustaches/schema.mustache")},
      m_include_mustache{
          readMustache("schema_parser/mustaches/include.mustache")},
      m_enum_mustache{readMustache("schema_parser/mustaches/enum.mustache")},
      m_enum_serd_header_mustache{
          readMustache("schema_parser/mustaches/enum_serd_header.mustache")},
      m_enum_serd_impl_mustache{
          readMustache("schema_parser/mustaches/enum_serd_impl.mustache")},
      m_schema_serd_header_mustache{
          readMustache("schema_parser/mustaches/schema_serd_header.mustache")},
      m_schema_serd_impl_mustache{
          readMustache("schema_parser/mustaches/schema_serd_impl.mustache")},
      m_class_serd_header_mustache{
          readMustache("schema_parser/mustaches/class_serd_header.mustache")},
      m_class_serd_impl_mustache{
          readMustache("schema_parser/mustaches/class_serd_impl.mustache")},
      m_asset_sl_header_mustache{
          readMustache("schema_parser/mustaches/asset_sl_header.mustache")},
      m_asset_sl_impl_mustache{
          readMustache("schema_parser/mustaches/asset_sl_impl.mustache")},
      m_asset_extension_mustache{
          readMustache("schema_parser/mustaches/asset_extension.mustache")} {}

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