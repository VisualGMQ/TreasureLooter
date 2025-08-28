#pragma once
#include "manager.hpp"
#include "schema/bind_point_schema.hpp"

#include <unordered_map>
#include <string>

struct BindPoints {
    std::unordered_map<std::string, BindPoint> m_bind_points;
};

class BindPointsComponentManager: public ComponentManager<BindPoints> {
public:
    void Update();
};
