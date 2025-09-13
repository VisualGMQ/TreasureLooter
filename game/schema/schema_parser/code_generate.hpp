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

std::string GenerateEnumDisplayHeaderCode(const EnumInfo&);
std::string GenerateEnumDisplayImplCode(const EnumInfo&);
std::string GenerateClassDisplayHeaderCode(const ClassInfo&);
std::string GenerateClassDisplayImplCode(const ClassInfo&);
std::string GenerateSchemaDisplayHeaderCode(const SchemaInfo&);
std::string GenerateSchemaDisplayImplCode(const SchemaInfo&);

// editor relative code
std::string GenerateAssetInfoHeaderCode(const SchemaInfoManager&);
std::string GenerateAssetInfoImplCode(const SchemaInfoManager&);
std::string GenerateAssetSerializeTotleHeaderCode(const SchemaInfoManager&);
std::string GenerateAssetDisplayTotleHeaderCode(const SchemaInfoManager&);
