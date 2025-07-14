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
