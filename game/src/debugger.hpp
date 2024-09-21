#pragma once
#include "pch.hpp"

namespace tl {

class DebugManager {
public:
    bool enableDrawGO = false;
    void Update();

private:
    void drawAllGO();
};

}  // namespace tl
