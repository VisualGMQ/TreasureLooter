#pragma once
#include "pch.hpp"
#include "go_hierarchy_watcher.hpp"
#include "inspector.hpp"

namespace tl {

class DebugManager {
public:
    bool enableDrawGO = false;
    bool simulateTouch = false;
    std::unique_ptr<GOHierarchyWatcher> hierarchyWatcher;
    std::unique_ptr<Inspector> inspector;

    DebugManager();
    void Update();

private:
    void drawAllGO();
};

}  // namespace tl
