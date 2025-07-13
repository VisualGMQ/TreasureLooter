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
      m_enum_mustache{readMustache("schema_parser/mustaches/enum.mustache")} {}

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