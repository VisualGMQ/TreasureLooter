#pragma once

#include "common.hpp"

#include <optional>
#include <rapidxml.hpp>

std::optional<EnumInfo> parseEnum(rapidxml::xml_node<>* node);
std::optional<std::string> parseInclude(rapidxml::xml_node<>* node);
std::optional<PropertyInfo> parseElement(rapidxml::xml_node<>* node);
std::optional<ClassInfo> parseClass(rapidxml::xml_node<>* node);
std::optional<SchemaInfo> parseSchema(const std::filesystem::path& filename);