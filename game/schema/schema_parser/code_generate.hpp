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

std::string GenerateProtoClassDeclareCode(const ClassInfo&);
std::string GenerateProtoEnumDeclareCode(const EnumInfo&);
std::string GenerateProtoSchemaDeclareCode(const SchemaInfo&);
std::string GenerateProtoAllProtoDeclareCode(const SchemaInfoManager&);
std::string GenerateProtoNetMsgDispatchHeaderCode(const SchemaInfoManager&);
std::string GenerateProtoNetMsgDispatchImplCode(const SchemaInfoManager&);
std::string GenerateProtoBindingHeaderCode(const SchemaInfoManager&);
std::string GenerateProtoBindingImplCode(const SchemaInfoManager&);
std::string GenerateProtoEventBindingHeaderCode(const SchemaInfoManager&);
std::string GenerateProtoEventBindingImplCode(const SchemaInfoManager&);
std::string GenerateProtoConvertHeaderCode(const SchemaInfoManager&);
std::string GenerateProtoConvertImplCode(const SchemaInfoManager&);

// editor relative code
std::string GenerateAssetInfoHeaderCode(const SchemaInfoManager&);
std::string GenerateAssetInfoImplCode(const SchemaInfoManager&);
std::string GenerateAssetSerializeTotleHeaderCode(const SchemaInfoManager&);
std::string GenerateAssetDisplayTotleHeaderCode(const SchemaInfoManager&);
std::string GenerateCppAssetExtensionHeaderCode(const SchemaInfoManager&);

// script binding code
std::string GenerateEnumScriptBindHeaderCode(const EnumInfo&);
std::string GenerateEnumScriptBindImplCode(const EnumInfo&);
std::string GenerateEnumStackSpecializationCode(const EnumInfo&);
std::string GenerateClassScriptBindHeaderCode(const ClassInfo&);
std::string GenerateClassScriptBindImplCode(const ClassInfo&);
std::string GenerateSchemaScriptBindHeaderCode(const SchemaInfo&);
std::string GenerateSchemaScriptBindImplCode(const SchemaInfo&);
std::string GenerateBindingHeaderCode(const SchemaInfoManager&);
std::string GenerateBindingImplCode(const SchemaInfoManager&);

// Luau definition file (declare-style, consumed by luau-lsp's types.definitionFiles):
// full content of scripts/type_hints/schema.d.luau (includes proto message types).
std::string GenerateSchemaTypesLuauDefinitionCode(const SchemaInfoManager&);
