#include "debugger.hpp"
#include "context.hpp"

namespace tl {

void DebugManager::Update() {
    if (enableDrawGO) {
        drawAllGO();
    }
}

void DebugManager::drawAllGO() {
    const float AxisLen = 50;
    auto& goMap = Context::GetInst().goMgr->GetAllGO();
    Renderer& renderer = *Context::GetInst().renderer;
    for (auto& [id, go] : goMap) {
        const Transform& transform = go.GetGlobalTransform(); 
        Vec2 xAxisEnd = Rotate(Vec2::X_AXIS * AxisLen * transform.scale.x, transform.rotation) + transform.position;
        Vec2 yAxisEnd = Rotate(Vec2::Y_AXIS * AxisLen * transform.scale.y, transform.rotation) + transform.position;
        renderer.DrawLine(transform.position, xAxisEnd, Color{255, 0, 0});
        renderer.DrawLine(transform.position, yAxisEnd, Color{0, 255, 0});
    }
}

}
