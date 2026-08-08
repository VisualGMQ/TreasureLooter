#include "context.hpp"

#include "client/animation_player.hpp"
#include "client/controller.hpp"
#include "client/debug_panel.hpp"
#include "client/detour.hpp"
#include "client/draw_order.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/gamepad.hpp"
#include "client/input/input.hpp"
#include "client/input/keyboard.hpp"
#include "client/input/mouse.hpp"
#include "client/message_box.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/debug_drawer.hpp"
#include "common/dialog.hpp"
#include "common/relationship.hpp"
#include "common/storage.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/trigger.hpp"
#include "imgui.h"
#include "instance_display.hpp"
#include "lyra/lyra.hpp"
#include "schema/display/display.hpp"
#include "schema/serialize/detour.hpp"
#include "variant_asset.hpp"

void TilemapDetourDataBakerContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<TilemapDetourDataBakerContext>(
            new TilemapDetourDataBakerContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void TilemapDetourDataBakerContext::Destroy() {
    instance.reset();
}

void TilemapDetourDataBakerContext::Initialize(int argc, char** argv) {
    ToolContext::Initialize(argc, argv);

    m_window->SetTitle("TreasureLooter TilemapDetourDataBaker");
    m_window->Resize({720, 680});

    FileDialog dialog{FileDialog::Type::OpenFile};
    dialog.AddFilter("tilemap", "tmx");
    dialog.AllowMultipleSelect(false);
    dialog.SetTitle("Select Tilemap");
    dialog.Open();
    auto files = dialog.GetSelectedFiles();
    TL_RETURN_IF_FALSE(!files.empty());

    m_tilemap = m_assets_manager->GetManager<Tilemap>().Load(files[0]);
}

void TilemapDetourDataBakerContext::update() {
    if (!m_tilemap) return;

    auto& layers = m_tilemap->GetLayers();

    if (m_layer_selected.size() != layers.size()) {
        m_layer_selected.resize(layers.size(), false);
    }

    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Tilemap Detour Data Builder", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    for (size_t i = 0; i < layers.size(); i++) {
        std::string label{layers[i]->GetName()};
        if (layers[i]->GetType() != TilemapLayer::Type::Tiled) {
            label += " (";
            label += layers[i]->GetType() == TilemapLayer::Type::Object
                         ? "Object"
                         : "Image";
            label += ")";
        }
        bool selected = m_layer_selected[i] != 0;
        ImGui::Checkbox(label.c_str(), &selected);
        m_layer_selected[i] = selected ? 1 : 0;
    }

    ImGui::Separator();

    if (ImGui::Button("Select All")) {
        for (size_t i = 0; i < layers.size(); i++) {
            m_layer_selected[i] = true;
        }
    }
    ImGui::SameLine();
    bool build_pressed = ImGui::Button("Build");

    ImGui::End();

    if (build_pressed) {
        auto handle = build();

        FileDialog dialog{FileDialog::Type::SaveFile};
        dialog.SetTitle("Save Tilemap Detour Data");
        dialog.AddFilter(
            {"Tilemap Detour Data",
             std::string{TilemapDetourData_AssetExtension.substr(1)}});
        dialog.SetDefaultFolder(GetProjectPath());
        dialog.Open();
        auto& files = dialog.GetSelectedFiles();
        if (!files.empty()) {
            Path filename = files[0];
            if (!filename.has_extension()) {
                filename.replace_extension(
                    TilemapDetourData_AssetExtension.substr(1));
            }
            SaveAsset(UUIDv4::CreateV4(), *handle, filename);
        }

        m_assets_manager->GetManager<TilemapDetourData>().Unload(handle);
        Exit();
    }
}

TilemapDetourDataHandle TilemapDetourDataBakerContext::build() {
    TL_RETURN_DEFAULT_IF_NULL(m_tilemap);

    int map_width = 0;
    int map_height = 0;
    auto& layers = m_tilemap->GetLayers();
    for (size_t i = 0; i < layers.size(); i++) {
        if (!m_layer_selected[i]) continue;
        if (layers[i]->GetType() != TilemapLayer::Type::Tiled) continue;
        auto* tiled = layers[i]->AsTiledLayer();
        auto& size = tiled->GetSize();
        map_width = std::max(map_width, static_cast<int>(size.x));
        map_height = std::max(map_height, static_cast<int>(size.y));
    }

    TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(
        map_width != 0 && map_height != 0, LOGI,
        "no layer select or layer extent is ZERO");

    auto tilemap_detour_data =
        m_assets_manager->GetManager<TilemapDetourData>().Create();
    tilemap_detour_data->m_data.Resize(map_width, map_height);

    auto& common_config = GetCommonConfig();
    for (size_t i = 0; i < layers.size(); i++) {
        if (!m_layer_selected[i]) continue;
        auto& layer = layers[i];
        auto tiled_layer = layer->AsTiledLayer();
        TL_CONTINUE_IF_NULL(tiled_layer);

        Entity entity = CreateEntity();
        TilemapLayerDefinition definition;
        definition.m_tilemap = m_tilemap;
        definition.m_layer_name = layer->GetName();
        m_tilemap_layer_collision_component_manager->RegisterEntity(
            entity, entity, definition, common_config.m_tile_in_chunk_size);
        auto tilemap_collision =
            m_tilemap_layer_collision_component_manager->Get(entity);

        auto collision = tilemap_collision->GetTilemapCollision();
        const auto& chunks = collision->m_chunks;

        auto size = tiled_layer->GetSize();
        int chunk_w = chunks.m_chunk_extent.w;
        int chunk_h = chunks.m_chunk_extent.h;

        for (int row = 0; row < size.h; row++) {
            for (int col = 0; col < size.w; col++) {
                int cx = col / chunk_w;
                int cy = row / chunk_h;
                int tx = col % chunk_w;
                int ty = row % chunk_h;

                if (cx < 0 || cy < 0 ||
                    static_cast<size_t>(cx) >= chunks.m_chunks.GetWidth() ||
                    static_cast<size_t>(cy) >= chunks.m_chunks.GetHeight()) {
                    continue;
                }

                auto& chunk = chunks.m_chunks.Get(cx, cy);
                if (tx < 0 || ty < 0 ||
                    static_cast<size_t>(tx) >= chunk.GetWidth() ||
                    static_cast<size_t>(ty) >= chunk.GetHeight()) {
                    continue;
                }

                DetourCostType weight = 1;
                if (!chunk.Get(tx, ty).empty()) {
                    weight = detour_inf_cost.GetUnderlyingValue();
                }
                tilemap_detour_data->m_data.Get(col, row) = weight;
            }
        }

        CLIENT_CONTEXT.RemoveEntity(entity);
    }

    return tilemap_detour_data;
}

TilemapDetourDataBakerContext& TilemapDetourDataBakerContext::GetInst() {
    return static_cast<TilemapDetourDataBakerContext&>(*instance);
}
