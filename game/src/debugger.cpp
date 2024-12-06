#include "debugger.hpp"
#include "context.hpp"
#include "macro.hpp"
#include "profile.hpp"

namespace tl {

DebugManager::DebugManager()
    : hierarchyWatcher{std::make_unique<GOHierarchyWatcher>()},
      inspector{std::make_unique<Inspector>()} {}

void DebugManager::Update() {
    PROFILE_FUNC(); 
    if (inspector) {
        inspector->Update();
    }
    if (hierarchyWatcher) {
        hierarchyWatcher->Update();
    }

    auto& scene = Context::GetInst().sceneMgr->GetCurScene();
    auto root = scene.GetRootGO();
    if (root) {
        updateRecurse(*root);
    }
}

void DebugManager::updateRecurse(GameObject& go) {
    if (enableDrawGO) {
        drawGO(go);
    }

    if (enableDrawCollisionShapes) {
        drawCollisionShape(go);
    }

    for (auto child : go.GetChildren()) {
        auto c =
            Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(child);
        TL_CONTINUE_IF_FALSE(c);
        updateRecurse(*c);
    }
}

void DebugManager::drawGO(const GameObject& go) {
    const float AxisLen = 50;
    Renderer& renderer = *Context::GetInst().renderer;
    const Transform& transform = go.GetGlobalTransform();
    Vec2 xAxisEnd = Rotate(Vec2::X_AXIS * AxisLen * transform.scale.x,
                            transform.rotation) +
                    transform.position;
    Vec2 yAxisEnd = Rotate(Vec2::Y_AXIS * AxisLen * transform.scale.y,
                            transform.rotation) +
                    transform.position;
    renderer.DrawLine(transform.position, xAxisEnd, Color{1, 0, 0});
    renderer.DrawLine(transform.position, yAxisEnd, Color{0, 1, 0});
}

void DebugManager::drawCollisionShape(const GameObject& go) {
    if (go.physicActor) {
        drawOneCollisionShape(go.physicActor.GetCollideShape());
    }

    TL_RETURN_IF_FALSE(go.tilemap);
    for (auto& layer : go.tilemap->GetLayers()) {
        if (layer->GetType() == MapLayerType::Tiles) {
            TileLayer* tileLayer = layer->AsTileLayer();
            for (int x = 0; x < tileLayer->GetSize().w; x++) {
                for (int y = 0; y < tileLayer->GetSize().h; y++) {
                    Tile* tile = tileLayer->GetTile(x, y);
                    TL_CONTINUE_IF_FALSE(tile->actor);
                    drawOneCollisionShape(tile->actor.GetCollideShape());
                }
            }
        }
    }
}

void DebugManager::drawOneCollisionShape(const Shape& shape) {
    auto& renderer = *Context::GetInst().renderer;
    switch (shape.type) {
        case Shape::Type::Unknown:
            break;
        case Shape::Type::AABB: {
            Rect rect;
            rect.position = shape.aabb.center - shape.aabb.halfSize;
            rect.size = shape.aabb.halfSize * 2.0;
            renderer.DrawRect(rect, Color::Red);
            break;
        }
        case Shape::Type::Circle: {
            renderer.DrawCircle(shape.circle, Color::Red);
            break;
        }
    }
}

} // namespace tl
