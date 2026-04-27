#pragma once
#include "common/manager.hpp"
#include "common/timer.hpp"
#include "schema/bind_point_schema.hpp"

#include <string>
#include <unordered_map>

struct BindPoint {
    std::string m_name;
    Vec2 m_position;

    Vec2 GetGlobalPosition() const;
    void UpdateGlobalPosition(const Transform& parent);

private:
    Vec2 m_global_position;
};

struct BindPoints {
    std::unordered_map<std::string, BindPoint> m_bind_points;

    BindPoints() = default;
    BindPoints(const std::vector<BindPointDefinition>&);
};

class BindPointsComponentManager : public ComponentManager<BindPoints> {
public:
    void Update();

    void ToggleDebugDraw();
    void RenderDebug(TimeType) const;
    bool IsEnableDebugDraw() const;

private:
    bool m_should_debug_draw = false;
};
