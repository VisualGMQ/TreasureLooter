#pragma once
#include "schema/input.hpp"

class Editor {
public:
    void Update();
    
private:
    bool m_open = true;
};