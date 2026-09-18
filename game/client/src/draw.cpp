#include "client/draw.hpp"

#include "client/asset_manager.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "client/ui.hpp"
#include "common/macros.hpp"
#include "common/profile.hpp"
#include "common/tilemap.hpp"

void DrawCommandSubmitter::Submit() {
    PROFILE_SECTION();

    auto level = CLIENT_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_NULL(level);

    submit(level->GetRootEntity());
}

void DrawCommandSubmitter::SubmitUI() {
    PROFILE_SECTION();

    auto level = CLIENT_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_NULL(level);

    auto relationship =
        CLIENT_CONTEXT.m_relationship_manager->Get(level->GetUIRootEntity());
    TL_RETURN_IF_NULL_WITH_LOG(
        relationship, LOGE,
        "[DrawCommandSubmitter]: ui root entity don't has relationship!");

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        submitUI(relationship->Get(i));
    }
}

void DrawCommandSubmitter::submit(LogicEntity root_entity) {
    submitRecursive(root_entity);
}

void DrawCommandSubmitter::submitUI(LogicEntity root_entity) {
    submitUIRecursive(root_entity);
}

void DrawCommandSubmitter::submitRecursive(LogicEntity entity) {
    PROFILE_SECTION();

    PresentEntity present_entity = ToPresentEntity(entity);

    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);

    auto draw_order =
        CLIENT_CONTEXT.m_draw_order_manager->Get(present_entity);

    bool should_enable_y_sorting =
        !CLIENT_CONTEXT.m_renderer->IsRecordingYSorting() && draw_order &&
        draw_order->IsEnableYSorting();
    if (should_enable_y_sorting) {
        CLIENT_CONTEXT.m_renderer->BeginYSorting();
    }

    CLIENT_CONTEXT.m_sprite_manager->SubmitDrawCommand(present_entity);
    CLIENT_CONTEXT.m_tilemap_layer_render_component_manager->SubmitDrawCommand(
        present_entity);

    TL_RETURN_IF_NULL(relationship);
    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        submitRecursive(relationship->Get(i));
    }

    if (should_enable_y_sorting) {
        CLIENT_CONTEXT.m_renderer->EndYSorting();
    }
}

void DrawCommandSubmitter::submitUIRecursive(LogicEntity entity) {
    PROFILE_SECTION();

    PresentEntity present_entity = ToPresentEntity(entity);

    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);

    auto draw_order =
        CLIENT_CONTEXT.m_draw_order_manager->Get(present_entity);

    bool should_enable_y_sorting =
        !CLIENT_CONTEXT.m_renderer->IsRecordingYSorting() && draw_order &&
        draw_order->IsEnableYSorting();
    if (should_enable_y_sorting) {
        CLIENT_CONTEXT.m_renderer->BeginYSorting();
    }

    CLIENT_CONTEXT.m_ui_manager->SubmitDrawCommand(entity);

    TL_RETURN_IF_NULL(relationship);
    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        submitUIRecursive(relationship->Get(i));
    }

    if (should_enable_y_sorting) {
        CLIENT_CONTEXT.m_renderer->EndYSorting();
    }
}
