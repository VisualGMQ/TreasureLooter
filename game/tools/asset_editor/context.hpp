#pragma once

#include "context.hpp"
#include "engine/context.hpp"
#include "variant_asset.hpp"

class AssetEditorContext : public CommonContext {
public:
    static void Init();
    static void Destroy();
    static AssetEditorContext& GetInst();

    AssetEditorContext(const AssetEditorContext&) = delete;
    AssetEditorContext& operator=(const AssetEditorContext&) = delete;
    AssetEditorContext(AssetEditorContext&&) = delete;
    AssetEditorContext& operator=(AssetEditorContext&&) = delete;
    ~AssetEditorContext() = default;

    void Initialize() override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void Update() override;

    void LoadAsset(Path);

private:
    static std::unique_ptr<AssetEditorContext> instance;
    Path m_project_path;
    VariantAsset m_asset;

    AssetEditorContext() = default;
    void parseProjectPath();
    void showMainMenu();

    void saveAs();

    void changeAssetPathInTitle(const Path& path);
};

#define ASSET_VIEWER_CONTEXT ::AssetEditorContext::GetInst()
