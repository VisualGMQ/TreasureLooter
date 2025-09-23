#pragma once
#include "quickjs.h"
#include <string>

class QJSEnumBase {
public:
    virtual const std::string& GetName() const = 0;
    virtual JSValue GetValue() const = 0;
    
    virtual ~QJSEnumBase() = default;
};