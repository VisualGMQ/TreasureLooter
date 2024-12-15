#include "level/test/test_physics.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl {

void TestPhysicsLevel::Enter() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = true;
}

void TestPhysicsLevel::Quit() {
    Context::GetInst().debugMgr->enableDrawCollisionShapes = false;
}

void DrawShape(const Shape& shape, const Vec2& dir, float t) {
    auto& renderer = Context::GetInst().renderer;
    switch (shape.type) {
        case Shape::Type::AABB: {
            AABB aabb = shape.aabb;
            aabb.center += dir * t;
            Rect rect;
            rect.position = aabb.center - aabb.halfSize;
            rect.size = aabb.halfSize * 2.0;
            renderer->DrawRect(rect, Color::Green);
        } break;
        case Shape::Type::Circle: {
            Circle c = shape.circle;
            c.center += dir * t;
            renderer->DrawCircle(c, Color::Green);
        } break;
    }
}

void TestPhysicsLevel::Update() {
    auto& mouse = Context::GetInst().mouse;
    if (mouse->GetButton(input::Mouse::Button::Type::Left).IsPressing()) {
        hoverPoint_ = mouse->GetPosition();
    }

    GameObject* go = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        Context::GetInst().debugMgr->hierarchyWatcher->GetSelected());
    TL_RETURN_IF_FALSE(go && go->physicActor);

    Vec2 pos = hoverPoint_;

    Shape shape = GetShapeRelateBy(*go);
    Vec2 dir = hoverPoint_ - shape.GetCenter();

    constexpr size_t HitInfoSize = 10;
    SweepHitInfo hitInfos[HitInfoSize];
    size_t count = Context::GetInst().physicsScene->Sweep(shape, dir, hitInfos,
                                                          HitInfoSize);
    for (int i = 0; i < count; i++) {
        const SweepHitInfo& info = hitInfos[i];
        if (info.t >= 0 && info.t <= 1) {
            DrawShape(shape, dir, info.t);
        }

        Context::GetInst().renderer->DrawLine(
            shape.aabb.center, shape.aabb.center + info.normal * 20,
            Color::Yellow);
    }

    Context::GetInst().renderer->DrawLine(shape.aabb.center, pos, Color::Blue);
}

}  // namespace tl