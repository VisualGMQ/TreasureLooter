#pragma once

#include "client/context.hpp"
#include "common/context.hpp"
#include "variant_asset.hpp"

class ToolContext : public ClientContext {
public:
    ToolContext(const ToolContext&) = delete;
    ToolContext& operator=(const ToolContext&) = delete;
    ToolContext(ToolContext&&) = delete;
    ToolContext& operator=(ToolContext&&) = delete;

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;

    void Update() final;

    Path ToProjectRelative(const Path& filename) const;
    const Path& GetProjectPath() const;

protected:
    using ClientContext::ClientContext;

    Path m_project_path;

    virtual void update() = 0;
    void parseProjectPath();
};
