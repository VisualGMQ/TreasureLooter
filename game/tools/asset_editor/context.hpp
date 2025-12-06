#pragma once

#include "engine/context.hpp"
#include "tool_context.hpp"

class AssetEditorContext : public ToolContext {
public:
    static void Init();
    static void Destroy();
    static AssetEditorContext& GetInst();

    void Initialize() override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void LoadAsset(Path);

protected:
    void update() override;

private:
    static std::unique_ptr<AssetEditorContext> instance;
    VariantAsset m_asset;

    void showMainMenu();
    void saveAs();
    void changeAssetPathInTitle(const Path& path);
};

#define ASSET_VIEWER_CONTEXT ::AssetEditorContext::GetInst()
