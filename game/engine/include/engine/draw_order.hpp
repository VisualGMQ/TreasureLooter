#pragma once

#include "engine/manager.hpp"
#include "schema/common.hpp"
#include "schema/draw_order.hpp"
#include <cstdint>

class DrawOrder {
public:
    friend class DrawOrderManager;

    DrawOrder() = default;
    explicit DrawOrder(const DrawOrderDefinition&);

    uint32_t m_z_order = 0;
    bool m_enable_y_sorting = false;

    double GetGlobalOrder() const;
    bool IsEnableYSorting() const;

private:
    double m_global_order = 0;
    bool m_inherit_y_sorting = false;
};

class DrawOrderManager : public ComponentManager<DrawOrder> {
public:
    void Update();

private:
    void updateRecursive(bool is_parent_enable_y_sorting, Entity entity);

    static constexpr uint32_t LayerFactor = 1000000;
    uint32_t m_id = 0;
};
