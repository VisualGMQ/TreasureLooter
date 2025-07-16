#pragma once

#include "common.hpp"

std::string GenerateClassCode(const ClassInfo&);
std::string GenerateSchemaCode(const SchemaInfo&);
std::string GenerateEnumCode(const EnumInfo&);

std::string GenerateEnumSerializeHeaderCode(const EnumInfo&);
std::string GenerateEnumSerializeImplCode(const EnumInfo&);
std::string GenerateClassSerializeHeaderCode(const ClassInfo&);
std::string GenerateClassSerializeImplCode(const ClassInfo&);
std::string GenerateSchemaSerializeHeaderCode(const SchemaInfo&);
std::string GenerateSchemaSerializeImplCode(const SchemaInfo&);
std::string GenerateAssetSLHeaderCode(const ClassInfo&);
std::string GenerateAssetSLImplCode(const ClassInfo&);

void GenerateSchemaAssetExtensionMustacheData(const SchemaInfo&,
                                              kainjow::mustache::data& out_data);
