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

    updateRecursive(false, level->GetRootEntity());
    updateRecursive(false, level->GetUIRootEntity());
}

void DrawOrderManager::updateRecursive(bool is_parent_enable_y_sorting,
                                       Entity entity) {
    auto order = Get(entity);
    if (order) {
        order->m_global_order = LayerFactor * order->m_z_order;
        order->m_inherit_y_sorting =
            order->m_enable_y_sorting || is_parent_enable_y_sorting;

        if (order->m_inherit_y_sorting) {
            order->m_global_order += m_id;
        } else {
            order->m_global_order += m_id++;
        }
    }

    if (auto relationship =
            CURRENT_CONTEXT.m_relationship_manager->Get(entity)) {
        for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
            updateRecursive(
                order ? order->m_inherit_y_sorting : is_parent_enable_y_sorting,
                relationship->Get(i));
        }
    }
}
