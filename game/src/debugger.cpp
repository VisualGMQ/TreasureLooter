#include "debugger.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl {

DebugManager::DebugManager()
    : hierarchyWatcher{std::make_unique<GOHierarchyWatcher>()},
      inspector{std::make_unique<Inspector>()} {}

void DebugManager::Update() {
    if (inspector) {
        inspector->Update();
    }
    if (hierarchyWatcher) {
        hierarchyWatcher->Update();
    }

    if (enableDrawGO) {
        drawAllGO();
    }
}

void DebugManager::drawAllGO() {
    const float AxisLen = 50;
    auto& goList = Context::GetInst().sceneMgr->GetCurScene()->GetAllGOID();
    Renderer& renderer = *Context::GetInst().renderer;
    for (auto id : goList) {
        auto go = Context::GetInst().goMgr->Find(id);
        TL_CONTINUE_IF(go);
        const Transform& transform = go->GetGlobalTransform();
        Vec2 xAxisEnd = Rotate(Vec2::X_AXIS * AxisLen * transform.scale.x,
                               transform.rotation) +
                        transform.position;
        Vec2 yAxisEnd = Rotate(Vec2::Y_AXIS * AxisLen * transform.scale.y,
                               transform.rotation) +
                        transform.position;
        renderer.DrawLine(transform.position, xAxisEnd, Color{255, 0, 0});
        renderer.DrawLine(transform.position, yAxisEnd, Color{0, 255, 0});
    }
}

}  // namespace tl
