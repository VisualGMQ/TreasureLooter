#pragma once

#include "engine/context.hpp"
#include "variant_asset.hpp"

class ToolContext : public CommonContext {
public:
    ToolContext(const ToolContext&) = delete;
    ToolContext& operator=(const ToolContext&) = delete;
    ToolContext(ToolContext&&) = delete;
    ToolContext& operator=(ToolContext&&) = delete;
    ~ToolContext() = default;

    void Initialize() override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void Update() final;

    void LoadAsset(Path);

protected:
    ToolContext() = default;
    virtual void update() = 0;

private:
    void parseProjectPath();
    void showMainMenu();

    void saveAs();

    void changeAssetPathInTitle(const Path& path);
};
