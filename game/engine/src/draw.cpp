#include "engine/draw.hpp"
#include "engine/context.hpp"
#include "engine/macros.hpp"
#include "engine/sprite.hpp"
#include "engine/tilemap.hpp"
#include "engine/ui.hpp"

void DrawCommandSubmitter::Submit() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    TL_RETURN_IF_NULL(level);

    submit(level->GetRootEntity());
}

void DrawCommandSubmitter::SubmitUI() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    TL_RETURN_IF_NULL(level);

    auto relationship =
        CURRENT_CONTEXT.m_relationship_manager->Get(level->GetUIRootEntity());
    TL_RETURN_IF_NULL_WITH_LOG(
        relationship, LOGE,
        "[DrawCommandSubmitter]: ui root entity don't has relationship!");

    for (auto child : relationship->m_children) {
        submitUI(child);
    }
}

void DrawCommandSubmitter::submit(Entity root_entity) {
    submitRecursive(root_entity);
}

void DrawCommandSubmitter::submitUI(Entity root_entity) {
    submitUIRecursive(root_entity);
}

void DrawCommandSubmitter::submitRecursive(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);

    auto draw_order = CURRENT_CONTEXT.m_draw_order_manager->Get(entity);

    bool should_enable_y_sorting =
        !CURRENT_CONTEXT.m_renderer->IsRecordingYSorting() && draw_order &&
        draw_order->IsEnableYSorting();
    if (should_enable_y_sorting) {
        CURRENT_CONTEXT.m_renderer->BeginYSorting();
    }

    CURRENT_CONTEXT.m_sprite_manager->SubmitDrawCommand(entity);
    CURRENT_CONTEXT.m_tilemap_layer_component_manager->SubmitDrawCommand(
        entity);

    TL_RETURN_IF_NULL(relationship);
    for (auto child : relationship->m_children) {
        submitRecursive(child);
    }

    if (should_enable_y_sorting) {
        CURRENT_CONTEXT.m_renderer->EndYSorting();
    }
}

void DrawCommandSubmitter::submitUIRecursive(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);

    auto draw_order = CURRENT_CONTEXT.m_draw_order_manager->Get(entity);

    bool should_enable_y_sorting =
        !CURRENT_CONTEXT.m_renderer->IsRecordingYSorting() && draw_order &&
        draw_order->IsEnableYSorting();
    if (should_enable_y_sorting) {
        CURRENT_CONTEXT.m_renderer->BeginYSorting();
    }

    CURRENT_CONTEXT.m_ui_manager->SubmitDrawCommand(entity);

    TL_RETURN_IF_NULL(relationship);
    for (auto child : relationship->m_children) {
        submitUIRecursive(child);
    }

    if (should_enable_y_sorting) {
        CURRENT_CONTEXT.m_renderer->EndYSorting();
    }
}
