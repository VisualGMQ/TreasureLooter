#pragma once

#include "common.hpp"

#include <string>

std::string generateClassCode(const ClassInfo&);
std::string generateSchemaCode(const SchemaInfo&);
std::string generateEnumCode(const EnumInfo&);
