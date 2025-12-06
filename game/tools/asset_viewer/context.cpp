#include "context.hpp"
#include "engine/storage.hpp"

std::unique_ptr<AssetViewerContext> AssetViewerContext::instance;

AssetViewerContext::AssetViewerContext() {}

void AssetViewerContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<AssetViewerContext>(new AssetViewerContext());
    } else {
        LOGW("inited context singleton twice!");
    }
}

void AssetViewerContext::Destroy() {
    instance.reset();
}

AssetViewerContext& AssetViewerContext::GetInst() {
    return *instance;
}

void AssetViewerContext::Initialize() {
    CommonContext::Initialize();

    m_window->SetTitle("TreasureLooter AssetViewer");
    m_window->Resize({720, 680});

    parseProjectPath();
}

void AssetViewerContext::parseProjectPath() {
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

    m_project_path = node->value();
}

void AssetViewerContext::Update() {
    showMainMenu();
}

void AssetViewerContext::showMainMenu() {

}

void AssetViewerContext::LoadAsset(const Path& filename) {
}
