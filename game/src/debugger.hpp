#pragma once
#include "pch.hpp"
#include "go_hierarchy_watcher.hpp"
#include "inspector.hpp"

namespace tl {

class DebugManager {
public:
    bool enableDrawGO = false;
    bool simulateTouch = false;
    bool enableDrawCollisionShapes = false;
    std::unique_ptr<GOHierarchyWatcher> hierarchyWatcher;
    std::unique_ptr<Inspector> inspector;

    DebugManager();
    void Update();

private:
    void drawGO(const GameObject&);
    void drawCollisionShape(const GameObject&);
    void drawOneCollisionShape(const Shape& shape);
    void updateRecurse(GameObject&);
};

}  // namespace tl
