#pragma once

#include "engine/entity.hpp"
#include "schema/gameplay_config.hpp"
#include "tool_context.hpp"
#include <memory>
#include <optional>

enum class CollisionAstKind {
    Weapon,
    Character,
};

class CollisionEditorContext : public ToolContext {
public:
    static void Init();
    static void Destroy();
    static CollisionEditorContext& GetInst();

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;
    void HandleEvents(const SDL_Event&) override;

protected:
    void update() override;

private:
    static std::unique_ptr<CollisionEditorContext> instance;

    std::optional<CollisionAstKind> m_kind;
    WeaponDefinitionHandle m_weapon;
    CharacterDefinitionHandle m_character;
    Path m_asset_path;

    Entity m_preview_entity = null_entity;
    int m_selected_hit_shape_index = -1;
    int m_prev_weapon_hit_shape_count = 0;

    void parseCmdArgs(int argc, char** argv);
    void showMainMenu();
    void showCollisionPanel();
    void showWeaponHitShapesUi();
    void changeWindowTitle(const Path& path);

    void loadAsset(Path absolute_path);
    void saveAsset();
    void saveAssetAs();
    void newWeaponAsset();
    void newCharacterAsset();
    void clearDocument();

    void clearPreviewEntity();
    void ensurePreviewSprite();
    void renderScenePreview();
};

#define COLLISION_EDITOR_CONTEXT ::CollisionEditorContext::GetInst()
