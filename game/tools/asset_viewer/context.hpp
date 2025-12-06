#pragma once

#include "context.hpp"
#include "engine/context.hpp"
#include "variant_asset.hpp"

class AssetViewerContext: public CommonContext {
public:
    static void Init();
    static void Destroy();
    static AssetViewerContext& GetInst();

    AssetViewerContext(const AssetViewerContext&) = delete;
    AssetViewerContext& operator=(const AssetViewerContext&) = delete;
    AssetViewerContext(AssetViewerContext&&) = delete;
    AssetViewerContext& operator=(AssetViewerContext&&) = delete;
    ~AssetViewerContext() override;

    void Initialize() override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void Update() override;

    void LoadAsset(const Path&);

private:
    static std::unique_ptr<AssetViewerContext> instance;
    Path m_project_path;
    VariantAsset m_asset;

    AssetViewerContext();
    void parseProjectPath();
    void showMainMenu();
};

#define ASSET_VIEWER_CONTEXT ::AssetViewerContext::GetInst()
