#include "tool_context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/relationship.hpp"
#include "engine/storage.hpp"
#include "imgui.h"
#include "schema/display/display.hpp"
#include "variant_asset.hpp"

void ToolContext::Initialize() {
    CommonContext::Initialize();
    parseProjectPath();
}

void ToolContext::Shutdown() {
    CommonContext::Shutdown();
}

void ToolContext::parseProjectPath() {
    auto file =
        IOStream::CreateFromFile("project_path.xml", IOMode::Read, true);
    if (!file) {
        return;
    }

    auto content = file->Read();
    content.push_back('\0');

    rapidxml::xml_document<> doc;
    doc.parse<0>(content.data());
    auto node = doc.first_node("ProjectPath");
    if (!node) {
        LOGE("no ProjectPath node in project_path.xml");
        return;
    }
}

struct DisplayAsset {
    void operator()(std::monostate) {}

    template <typename T>
    void operator()(T handle) {
        using payload_type = typename T::underlying_type;
        InstanceDisplay(AssetInfoManager::GetName<payload_type>().data(),
                        *handle);
    }
};

void ToolContext::Update() {
    m_renderer->Clear();
    beginImGui();

    update();

    endImGui();
    m_renderer->Present();
}

void ToolContext::HandleEvents(const SDL_Event& event) {
    CommonContext::HandleEvents(event);
}
