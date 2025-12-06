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

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void Update() final;

    Path ToProjectRelative(const Path& filename) {
        std::error_code err;
        auto& project_path = GetProjectPath();
        auto relative = std::filesystem::relative(filename, project_path, err);
        if (err) {
            LOGE("path {} not related to project path {}", filename, project_path);
            return filename;
        }
        auto str = relative.string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return str;
    }

protected:
    ToolContext() = default;
    virtual void update() = 0;
    void parseProjectPath() override;
};
