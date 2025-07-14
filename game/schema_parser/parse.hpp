#pragma once

#include "common.hpp"

#include <optional>
#include <rapidxml.hpp>

std::optional<EnumInfo> ParseEnum(rapidxml::xml_node<>* node);
std::optional<std::string> ParseInclude(rapidxml::xml_node<>* node);
std::optional<PropertyInfo> ParseElement(rapidxml::xml_node<>* node);
std::optional<PropertyInfo> ParseOption(SchemaInfo&, rapidxml::xml_node<>* node);
std::optional<PropertyInfo> ParseArray(SchemaInfo&, rapidxml::xml_node<>* node);
std::optional<PropertyInfo> ParseUnorderedMap(SchemaInfo&, rapidxml::xml_node<>* node);
std::optional<ClassInfo> ParseClass(SchemaInfo&, rapidxml::xml_node<>* node);
std::optional<SchemaInfo> ParseSchema(const std::filesystem::path& filename);