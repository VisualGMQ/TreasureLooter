#include "client/animation_player.hpp"
#include "client/camera.hpp"
#include "client/controller.hpp"
#include "client/draw.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/input.hpp"
#include "client/input/keyboard.hpp"
#include "client/input/mouse.hpp"
#include "client/renderer.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "common/asset_manager.hpp"
#include "common/cct.hpp"
#include "common/debug_drawer.hpp"
#include "common/dialog.hpp"
#include "common/relationship.hpp"
#include "common/storage.hpp"
#include "common/trigger.hpp"
#include "instance_display.hpp"
#include "lyra/lyra.hpp"
#include "rapidxml.hpp"
#include "schema/display/physics_schema.hpp"
#include "schema/physics_schema.hpp"
#include "schema/serialize/gameplay_config.hpp"
#include "schema/serialize/physics_schema.hpp"

#include "imgui.h"

#include "context.hpp"

#include "client/draw_order.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

namespace {

const Color kWeaponHitUnselOutline{0.35f, 0.75f, 0.95f, 1.0f};
const Color kWeaponHitUnselFill{0.35f, 0.75f, 0.95f, 0.12f};
const Color kWeaponHitSelOutline{0.55f, 0.35f, 0.12f, 1.0f};
const Color kWeaponHitSelFill{0.55f, 0.35f, 0.12f, 0.22f};

}  // namespace

std::unique_ptr<CollisionEditorContext> CollisionEditorContext::instance;

bool pathFilenameEndsWith(const Path& p, std::string_view suffix) {
    const std::string fn = p.filename().string();
    return fn.size() >= suffix.size() &&
           std::string_view(fn).substr(fn.size() - suffix.size()) == suffix;
}

std::optional<CollisionAstKind> detectCollisionAstKindFromXmlRoot(
    const Path& path) {
    auto file = IOStream::CreateFromFile(path, IOMode::Read, true);
    if (!file) {
        return std::nullopt;
    }
    auto content = file->Read();
    content.push_back('\0');
    rapidxml::xml_document<> doc;
    try {
        doc.parse<rapidxml::parse_default>(content.data());
    } catch (...) {
        return std::nullopt;
    }
    if (doc.first_node("WeaponDefinition")) {
        return CollisionAstKind::Weapon;
    }
    if (doc.first_node("CharacterDefinition")) {
        return CollisionAstKind::Character;
    }
    return std::nullopt;
}

std::optional<CollisionAstKind> detectCollisionAstKind(
    const Path& absolute_path) {
    if (pathFilenameEndsWith(absolute_path, ".weapon_def.xml")) {
        return CollisionAstKind::Weapon;
    }
    if (pathFilenameEndsWith(absolute_path, ".character_def.xml")) {
        return CollisionAstKind::Character;
    }
    return detectCollisionAstKindFromXmlRoot(absolute_path);
}

void SaveExternalPhysicsActorIfNeeded(
    const PhysicsShapeDefinitionHandle& handle) {
    if (!handle || handle.IsEmbed()) {
        return;
    }
    auto filename = handle.GetFilename();
    if (!filename || filename->empty()) {
        return;
    }
    SaveAsset(handle.GetUUID(), *handle, *filename);
}

void SaveReferencedExternalPhysicsActors(const WeaponDefinition& weapon) {
    for (const auto& shape_handle : weapon.m_hit_shapes.m_physics_shapes) {
        SaveExternalPhysicsActorIfNeeded(shape_handle);
    }
}

void SaveReferencedExternalPhysicsActors(const CharacterDefinition& character) {
    SaveExternalPhysicsActorIfNeeded(character.m_cct.m_physics_shape);
}

void drawMissingPhysicsPlaceholder(Renderer& renderer, Vec2 pos, float z_base,
                                   const Color& edge) {
    constexpr float kHalf = 6.0f;
    Rect r;
    r.m_center = pos;
    r.m_half_size = {kHalf, kHalf};
    renderer.FillRect(
        r, Color{edge.r * 0.35f, edge.g * 0.35f, edge.b * 0.35f, 0.45f}, z_base,
        true);
    renderer.DrawRect(r, edge, z_base + 1.0f, true);
}

void drawPhysicsShapeForPreview(Renderer& renderer,
                                const PhysicsShapeDefinition& info,
                                float z_base, const Color& outline,
                                const Color& fill) {
    constexpr bool kUseCamera = true;
    const bool rect_has_area =
        info.m_rect.m_half_size.x > 0.0f || info.m_rect.m_half_size.y > 0.0f;
    const bool circle_has_radius = info.m_circle.m_radius > 0.0f;

    if (info.m_is_rect && rect_has_area) {
        Rect r = info.m_rect;
        renderer.FillRect(r, fill, z_base, kUseCamera);
        renderer.DrawRect(r, outline, z_base + 1.0f, kUseCamera);
        return;
    }

    // Circle geometry: explicit circle, or inconsistent data (rect flag but no
    // rect half_size while circle radius is valid).
    if ((!info.m_is_rect && circle_has_radius) ||
        (info.m_is_rect && !rect_has_area && circle_has_radius)) {
        const Circle c = info.m_circle;
        renderer.DrawCircle(c, outline, 48, z_base + 1.0f, kUseCamera);
        return;
    }

    const Vec2 center =
        info.m_is_rect ? info.m_rect.m_center : info.m_circle.m_center;
    drawMissingPhysicsPlaceholder(renderer, center, z_base, outline);
}

void drawReferenceImageForPreview(Renderer& renderer, const ImageBase& image,
                                  float z_order) {
    const Vec2 size = image.GetSize();
    const Vec2 half = size * 0.5f;
    const Region src{
        {0.0f, 0.0f},
        size
    };
    renderer.DrawImageEx(image, src, Vec2{-half.x, -half.y},
                         Vec2{half.x, -half.y}, Vec2{-half.x, half.y},
                         Color::White, z_order, true);
}

void drawReferenceImageForPreview(Renderer& renderer,
                                  const SpriteDefinition& sprite,
                                  float z_order) {
    if (!sprite.m_image) {
        return;
    }
    Region src_region = sprite.m_region;
    if (src_region.m_size.w == 0 || src_region.m_size.h == 0) {
        src_region.m_size = sprite.m_image->GetSize();
    }

    Vec2 half_size = sprite.m_region.m_size * 0.5f;
    if (half_size.x == 0 || half_size.y == 0) {
        half_size = sprite.m_image->GetSize() * 0.5f;
    }

    std::array<Vec2, 3> pts;
    pts[0] = -half_size;                   // top left
    pts[1] = {half_size.x, -half_size.y};  // top right
    pts[2] = {-half_size.x, half_size.y};  // bottom left

    if (sprite.m_flip & Flip::Horizontal && sprite.m_flip & Flip::Vertical) {
        pts[0] = half_size;
        pts[1] = {-half_size.x, half_size.y};
        pts[2] = {half_size.x, -half_size.y};
    } else if (sprite.m_flip & Flip::Horizontal) {
        pts[0] = {half_size.x, -half_size.y};
        pts[1] = {-half_size.x, -half_size.y};
        pts[2] = {half_size.x, half_size.y};
    } else if (sprite.m_flip & Flip::Vertical) {
        pts[0] = {-half_size.x, half_size.y};
        pts[1] = {half_size.x, half_size.y};
        pts[2] = -half_size;
    }

    for (auto& pt : pts) {
        pt -= sprite.m_anchor;
    }

    renderer.DrawImageEx(*sprite.m_image, src_region, pts[0], pts[1], pts[2],
                         sprite.m_color, z_order, true);
}

void CollisionEditorContext::Init() {
    if (!instance) {
        instance =
            std::unique_ptr<CollisionEditorContext>(new CollisionEditorContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void CollisionEditorContext::Destroy() {
    instance.reset();
}

CollisionEditorContext& CollisionEditorContext::GetInst() {
    return *instance;
}

void CollisionEditorContext::Initialize(int argc, char** argv) {
    ToolContext::Initialize(argc, argv);

    m_window->SetTitle("TreasureLooter CollisionEditor - [No Name]");
    m_window->Resize({960, 720});
    parseCmdArgs(argc, argv);
}

void CollisionEditorContext::Shutdown() {
    clearDocument();
    ToolContext::Shutdown();
}

void CollisionEditorContext::HandleEvents(const SDL_Event& event) {
    ToolContext::HandleEvents(event);
}

void CollisionEditorContext::parseCmdArgs(int argc, char** argv) {
    std::filesystem::path filename;
    auto cli = lyra::cli() | lyra::opt(filename, "filename")["--filename"];
    lyra::parse_result result = cli.parse({argc, argv});
    if (!result) {
        LOGE("Command line parse failed: {}", result.message());
        return;
    }
    if (std::filesystem::is_regular_file(filename)) {
        loadAsset(Path{filename.string()});
    }
}

void CollisionEditorContext::changeWindowTitle(const Path& path) {
    if (path.empty()) {
        m_window->SetTitle("TreasureLooter CollisionEditor - [No Name]");
    } else {
        m_window->SetTitle("TreasureLooter CollisionEditor - " + path.string());
    }
}

void CollisionEditorContext::clearDocument() {
    clearPreviewEntity();
    m_weapon.Reset();
    m_character.Reset();
    m_kind.reset();
    m_asset_path.clear();
    m_selected_hit_shape_index = -1;
    m_prev_weapon_physics_shape_count = 0;
    changeWindowTitle({});
}

void CollisionEditorContext::loadAsset(Path absolute_path) {
    auto kind = detectCollisionAstKind(absolute_path);
    if (!kind) {
        LOGE("Not a collision editor document: open a *.weapon_def.xml or "
             "*.character_def.xml file, "
             "or XML with <WeaponDefinition> / <CharacterDefinition> root.");
        return;
    }

    auto relative = ToProjectRelative(absolute_path);
    if (*kind == CollisionAstKind::Weapon) {
        m_character.Reset();
        auto h = m_assets_manager->GetManager<WeaponDefinition>().Load(relative,
                                                                       true);
        if (!h) {
            LOGE("Failed to load WeaponDefinition: {}", relative.string());
            return;
        }
        m_weapon = std::move(h);
        m_kind = kind;
        m_asset_path = relative;
        changeWindowTitle(relative);
        if (!m_weapon->m_hit_shapes.m_physics_shapes.empty()) {
            m_selected_hit_shape_index = 0;
        } else {
            m_selected_hit_shape_index = -1;
        }
        m_prev_weapon_physics_shape_count =
            static_cast<int>(m_weapon->m_hit_shapes.m_physics_shapes.size());
    } else {
        m_weapon.Reset();
        auto h = m_assets_manager->GetManager<CharacterDefinition>().Load(
            relative, true);
        if (!h) {
            LOGE("Failed to load CharacterDefinition: {}", relative.string());
            return;
        }
        m_character = std::move(h);
        m_kind = kind;
        m_asset_path = relative;
        changeWindowTitle(relative);
        m_selected_hit_shape_index = -1;
        m_prev_weapon_physics_shape_count = 0;
    }
}

void CollisionEditorContext::saveAsset() {
    if (!m_kind) {
        return;
    }
    if (m_asset_path.empty()) {
        saveAssetAs();
        return;
    }
    if (*m_kind == CollisionAstKind::Weapon) {
        if (!m_weapon) {
            return;
        }
        SaveReferencedExternalPhysicsActors(*m_weapon);
        SaveAsset(m_weapon.GetUUID(), *m_weapon, m_asset_path);
        loadAsset(GetProjectPath() / m_asset_path);
    } else {
        if (!m_character) {
            return;
        }
        SaveReferencedExternalPhysicsActors(*m_character);
        SaveAsset(m_character.GetUUID(), *m_character, m_asset_path);
        loadAsset(GetProjectPath() / m_asset_path);
    }
}

void CollisionEditorContext::saveAssetAs() {
    if (!m_kind) {
        return;
    }

    FileDialog dialog{FileDialog::Type::SaveFile};
    dialog.SetTitle("Save As");
    if (*m_kind == CollisionAstKind::Weapon) {
        dialog.AddFilter({"WeaponDefinition", "weapon_def.xml"});
    } else {
        dialog.AddFilter({"CharacterDefinition", "character_def.xml"});
    }
    dialog.SetDefaultFolder(GetProjectPath());
    dialog.Open();
    auto& files = dialog.GetSelectedFiles();
    if (files.empty()) {
        return;
    }
    Path final_file = files[0];
    auto extension = final_file.extension().string();
    if (extension.empty()) {
        if (*m_kind == CollisionAstKind::Weapon) {
            final_file += ".weapon_def.xml";
        } else {
            final_file += ".character_def.xml";
        }
    }

    auto relative = ToProjectRelative(final_file);
    if (*m_kind == CollisionAstKind::Weapon) {
        if (!m_weapon) {
            return;
        }
        SaveReferencedExternalPhysicsActors(*m_weapon);
        SaveAsset(m_weapon.GetUUID(), *m_weapon, relative);
        loadAsset(GetProjectPath() / relative);
    } else {
        if (!m_character) {
            return;
        }
        SaveReferencedExternalPhysicsActors(*m_character);
        SaveAsset(m_character.GetUUID(), *m_character, relative);
        loadAsset(GetProjectPath() / relative);
    }
}

void CollisionEditorContext::newWeaponAsset() {
    clearPreviewEntity();
    m_character.Reset();
    m_weapon = m_assets_manager->GetManager<WeaponDefinition>().Create();
    m_kind = CollisionAstKind::Weapon;
    m_asset_path.clear();
    m_weapon->m_hit_shapes.m_event_type = TriggerEventType::WeaponAttack;
    m_selected_hit_shape_index = -1;
    m_prev_weapon_physics_shape_count = 0;
    changeWindowTitle({});
}

void CollisionEditorContext::newCharacterAsset() {
    clearPreviewEntity();
    m_weapon.Reset();
    m_character = m_assets_manager->GetManager<CharacterDefinition>().Create();
    m_kind = CollisionAstKind::Character;
    m_asset_path.clear();
    m_selected_hit_shape_index = -1;
    m_prev_weapon_physics_shape_count = 0;
    changeWindowTitle({});
}

void CollisionEditorContext::showMainMenu() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Weapon (hit_shapes)")) {
            newWeaponAsset();
        }
        if (ImGui::MenuItem("New Character (cct)")) {
            newCharacterAsset();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Open...")) {
            FileDialog dialog{FileDialog::Type::OpenFile};
            dialog.SetTitle("Open");
            dialog.AddFilter({"WeaponDefinition", "weapon_def.xml"});
            dialog.AddFilter({"CharacterDefinition", "character_def.xml"});
            dialog.SetDefaultFolder(GetProjectPath());
            dialog.Open();
            auto& files = dialog.GetSelectedFiles();
            if (!files.empty()) {
                loadAsset(files[0]);
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save", nullptr, false,
                            static_cast<bool>(m_kind))) {
            saveAsset();
        }
        if (ImGui::MenuItem("Save As...", nullptr, false,
                            static_cast<bool>(m_kind))) {
            saveAssetAs();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void CollisionEditorContext::showWeaponHitShapesUi() {
    auto& weapon = *m_weapon;
    auto& hit_trigger = weapon.m_hit_shapes;

    ImGui::SeparatorText("Hit shapes (one trigger)");
    InstanceDisplay("event_type", hit_trigger.m_event_type);
    InstanceDisplay("trig_every_frame_when_touch",
                    hit_trigger.m_trig_every_frame_when_touch);

    if (ImGui::Button("Add physics shape")) {
        auto h =
            m_assets_manager->GetManager<PhysicsShapeDefinition>().Create();
        hit_trigger.m_physics_shapes.push_back(std::move(h));
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove selected shape")) {
        auto& shapes = hit_trigger.m_physics_shapes;
        const int n = static_cast<int>(shapes.size());
        if (n > 0 && m_selected_hit_shape_index >= 0 &&
            m_selected_hit_shape_index < n) {
            const size_t i = static_cast<size_t>(m_selected_hit_shape_index);
            const int old_sel = m_selected_hit_shape_index;
            shapes.erase(shapes.begin() + static_cast<ptrdiff_t>(i));
            const int new_n = static_cast<int>(shapes.size());
            if (new_n == 0) {
                m_selected_hit_shape_index = -1;
            } else if (old_sel == static_cast<int>(i)) {
                m_selected_hit_shape_index =
                    std::min(static_cast<int>(i), new_n - 1);
            } else if (old_sel > static_cast<int>(i)) {
                m_selected_hit_shape_index--;
            }
        }
    }

    auto& shapes = hit_trigger.m_physics_shapes;
    const int n = static_cast<int>(shapes.size());
    if (n > m_prev_weapon_physics_shape_count) {
        m_selected_hit_shape_index = n - 1;
    }
    m_prev_weapon_physics_shape_count = n;
    if (n == 0) {
        m_selected_hit_shape_index = -1;
    } else if (m_selected_hit_shape_index < 0 ||
               m_selected_hit_shape_index >= n) {
        m_selected_hit_shape_index = 0;
    }

    for (int i = 0; i < n; ++i) {
        ImGui::PushID(i);
        auto& shape_handle = shapes[static_cast<size_t>(i)];
        char title[64];
        std::snprintf(title, sizeof(title), "Physics shape %d###hs%d", i, i);

        const bool node_selected = (m_selected_hit_shape_index == i);
        ImGuiTreeNodeFlags node_flags =
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (node_selected) {
            node_flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::PushStyleColor(
                ImGuiCol_Header,
                ImVec4(kWeaponHitSelOutline.r, kWeaponHitSelOutline.g,
                       kWeaponHitSelOutline.b, 0.55f));
        }
        const bool open = ImGui::TreeNodeEx(title, node_flags);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            m_selected_hit_shape_index = i;
        }
        if (node_selected) {
            ImGui::PopStyleColor();
        }
        if (open) {
            InstanceDisplay("shape", shape_handle);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void CollisionEditorContext::showCollisionPanel() {
    if (!ImGui::Begin("Collision")) {
        ImGui::End();
        return;
    }

    if (!m_kind) {
        ImGui::TextWrapped("No document. Use File → Open or create New. "
                           "Supported: *.weapon_def.xml (hit shapes), "
                           "*.character_def.xml (CCT)");
        ImGui::End();
        return;
    }

    if (*m_kind == CollisionAstKind::Weapon) {
        ImGui::TextUnformatted("WeaponDefinition");
    } else {
        ImGui::TextUnformatted("CharacterDefinition");
    }
    if (!m_asset_path.empty()) {
        ImGui::Text("Path: %s", m_asset_path.string().c_str());
    } else {
        ImGui::TextUnformatted("Path: <unsaved>");
    }
    ImGui::Spacing();

    if (*m_kind == CollisionAstKind::Weapon) {
        if (!m_weapon) {
            ImGui::TextUnformatted("Internal error: no weapon asset.");
            ImGui::End();
            return;
        }
        ImGui::SeparatorText("WeaponDefinition");
        InstanceDisplay("sprite", m_weapon->m_sprite);
        showWeaponHitShapesUi();
    } else {
        if (!m_character) {
            ImGui::TextUnformatted("Internal error: no character asset.");
            ImGui::End();
            return;
        }
        ImGui::SeparatorText("CharacterDefinition");
        InstanceDisplay("sprite_sheet", m_character->m_sprite_sheet);
        ImGui::SeparatorText("CharacterDefinition — CCTDefinition");
        InstanceDisplay("cct", m_character->m_cct);
    }

    ImGui::End();
}

void CollisionEditorContext::clearPreviewEntity() {
    if (m_preview_entity == null_entity) {
        return;
    }
    m_sprite_manager->RemoveEntity(m_preview_entity);
    m_transform_manager->RemoveEntity(m_preview_entity);
    m_preview_entity = null_entity;
}

void CollisionEditorContext::ensurePreviewSprite() {
    if (!m_kind) {
        clearPreviewEntity();
        return;
    }
    ImageHandle img;
    const SpriteDefinition* weapon_sprite = nullptr;
    if (*m_kind == CollisionAstKind::Weapon) {
        if (!m_weapon) {
            clearPreviewEntity();
            return;
        }
        weapon_sprite = &m_weapon->m_sprite;
        img = m_weapon->m_sprite.m_image;
    } else {
        if (!m_character) {
            clearPreviewEntity();
            return;
        }
        img = m_character->m_sprite_sheet;
    }
    if (!img) {
        clearPreviewEntity();
        return;
    }
    if (m_preview_entity == null_entity) {
        m_preview_entity = CreateEntity();
        m_transform_manager->ReplaceComponent(m_preview_entity, Transform{});
    }
    Sprite* spr = m_sprite_manager->Get(m_preview_entity);
    const bool need_weapon_sprite_sync =
        (*m_kind == CollisionAstKind::Weapon && weapon_sprite != nullptr);
    if (need_weapon_sprite_sync || !spr ||
        spr->m_image.GetUUID() != img.GetUUID()) {
        Sprite sprite{};
        if (*m_kind == CollisionAstKind::Weapon && weapon_sprite) {
            sprite = *weapon_sprite;
        } else {
            sprite.m_image = img;
            const Vec2 sz = img->GetSize();
            sprite.m_region.m_topleft = {0.0f, 0.0f};
            sprite.m_region.m_size = sz;
        }
        m_sprite_manager->ReplaceComponent(m_preview_entity, sprite);
    }
}

void CollisionEditorContext::renderScenePreview() {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            Vec2 scale = m_camera.GetScale();
            scale.x = std::max(scale.x, 0.001f);
            scale.y = std::max(scale.y, 0.001f);
            m_camera.Move(
                {-io.MouseDelta.x / scale.x, -io.MouseDelta.y / scale.y});
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            float factor = 1.0f + (-io.MouseDelta.y * 0.01f);
            factor = std::max(0.2f, factor);
            Vec2 scale = m_camera.GetScale() * factor;
            scale.x = Clamp(scale.x, 0.05f, 20.0f);
            scale.y = Clamp(scale.y, 0.05f, 20.0f);
            m_camera.ChangeScale(scale);
        }
        if (io.MouseWheel != 0.0f) {
            float factor = 1.0f + (io.MouseWheel * 0.1f);
            Vec2 scale = m_camera.GetScale() * factor;
            scale.x = Clamp(scale.x, 0.05f, 20.0f);
            scale.y = Clamp(scale.y, 0.05f, 20.0f);
            m_camera.ChangeScale(scale);
        }
    }

    constexpr float axis_len = 10000.0f;
    m_renderer->DrawLine({-axis_len, 0.0f}, {axis_len, 0.0f},
                         Color{1.0f, 0.2f, 0.2f, 0.8f}, -1000.0f, true);
    m_renderer->DrawLine({0.0f, -axis_len}, {0.0f, axis_len},
                         Color{0.2f, 1.0f, 0.2f, 0.8f}, -1000.0f, true);

    m_relationship_manager->Update();
    m_draw_order_manager->Update();

    DrawCommandSubmitter draw_cmd_submitter;
    draw_cmd_submitter.Submit();

    // z: reference image at 0, hit shapes above it.
    constexpr float kReferenceImageZ = 0.0f;
    constexpr float kShapeZ = 50.0f;

    if (m_kind && *m_kind == CollisionAstKind::Weapon && m_weapon &&
        m_weapon->m_sprite.m_image) {
        drawReferenceImageForPreview(*m_renderer, m_weapon->m_sprite,
                                     kReferenceImageZ);
    } else if (m_kind && *m_kind == CollisionAstKind::Character &&
               m_character && m_character->m_sprite_sheet) {
        drawReferenceImageForPreview(*m_renderer, *m_character->m_sprite_sheet,
                                     kReferenceImageZ);
    }

    if (m_kind && *m_kind == CollisionAstKind::Weapon && m_weapon) {
        const auto& trig = m_weapon->m_hit_shapes;
        const int n = static_cast<int>(trig.m_physics_shapes.size());
        for (int i = 0; i < n; ++i) {
            const auto& h = trig.m_physics_shapes[static_cast<size_t>(i)];
            const bool is_sel = (m_selected_hit_shape_index == i);
            const Color& outline =
                is_sel ? kWeaponHitSelOutline : kWeaponHitUnselOutline;
            const Color& fill =
                is_sel ? kWeaponHitSelFill : kWeaponHitUnselFill;
            const float z = kShapeZ + static_cast<float>(i) * 0.5f;
            if (h) {
                drawPhysicsShapeForPreview(*m_renderer, *h, z, outline, fill);
            } else {
                drawMissingPhysicsPlaceholder(*m_renderer, Vec2{0.0f, 0.0f}, z,
                                              outline);
            }
        }
        if (n == 0) {
            drawMissingPhysicsPlaceholder(*m_renderer, Vec2{0.0f, 0.0f},
                                          kShapeZ, kWeaponHitUnselOutline);
        }
    } else if (m_kind && *m_kind == CollisionAstKind::Character &&
               m_character && m_character->m_cct.m_physics_shape) {
        const Color cct_outline{0.35f, 0.85f, 0.45f, 1.0f};
        const Color cct_fill{0.35f, 0.85f, 0.45f, 0.18f};
        drawPhysicsShapeForPreview(*m_renderer,
                                   *m_character->m_cct.m_physics_shape, kShapeZ,
                                   cct_outline, cct_fill);
    }

    m_renderer->ApplyDrawcall();
}

void CollisionEditorContext::update() {
#ifdef IMGUI_HAS_DOCK
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);
#endif
    showMainMenu();
    showCollisionPanel();
    renderScenePreview();
}
