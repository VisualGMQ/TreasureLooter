#include "engine/draw_order.hpp"
#include "engine/context.hpp"
#include "engine/profile.hpp"
#include "engine/relationship.hpp"

DrawOrder::DrawOrder(const DrawOrderDefinition& def)
    : m_z_order{static_cast<uint32_t>(def.m_draw_layer)},
      m_enable_y_sorting{def.m_enable_y_sorting} {}

double DrawOrder::GetGlobalOrder() const {
    return m_global_order;
}

bool DrawOrder::IsEnableYSorting() const {
    return m_enable_y_sorting || m_inherit_y_sorting;
}

void DrawOrderManager::Update() {
    PROFILE_SECTION();

    auto level = CURRENT_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_NULL(level);

    update(level->GetRootEntity());
    update(level->GetUIRootEntity());
}

void DrawOrderManager::update(Entity entity) {
    m_id = 0;

    auto relationship =
        CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_NULL(relationship);
    auto order = CURRENT_CONTEXT.m_draw_order_manager->Get(entity);

    for (auto child : relationship->m_children) {
        updateRecursive(order, child);
    }
}

void DrawOrderManager::updateRecursive(const DrawOrder* parent_order,
                                       Entity entity) {
    auto order = Get(entity);
    TL_RETURN_IF_NULL(order);

    order->m_global_order = LayerFactor * order->m_z_order;
    if (parent_order) {
        order->m_inherit_y_sorting =
            order->m_enable_y_sorting || parent_order->m_inherit_y_sorting;
    } else {
        order->m_inherit_y_sorting = order->m_enable_y_sorting;
    }

    if (order->m_inherit_y_sorting) {
        order->m_global_order += m_id;
    } else {
        order->m_global_order += m_id++;
    }

    if (auto relationship =
            CURRENT_CONTEXT.m_relationship_manager->Get(entity)) {
        for (auto child : relationship->m_children) {
            updateRecursive(order, child);
        }
    }
}
