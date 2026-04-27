#include "client/scene.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/ui.hpp"
#include "client/window.hpp"
#include "common/asset_manager.hpp"

void ClientScene::OnEnter() {
    Scene::OnEnter();

    Transform* transform =
        COMMON_CONTEXT.m_transform_manager->Get(GetUIRootEntity());
    transform->m_size = CLIENT_CONTEXT.m_window->GetWindowSize();

    m_window_resize_event_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
            [this](EventListenerID, const SDL_WindowEvent& event) {
                if (event.type != SDL_EVENT_WINDOW_RESIZED &&
                    event.type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED &&
                    event.type != SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
                    return;
                }
                Entity entity = this->GetUIRootEntity();
                Transform* transform =
                    COMMON_CONTEXT.m_transform_manager->Get(entity);
                if (!(event.data1 == 0 && event.data2 == 0)) {
                    transform->m_size.w = event.data1;
                    transform->m_size.h = event.data2;
                }
            });
}

void ClientScene::OnQuit() {
    COMMON_CONTEXT.m_event_system->RemoveListener<SDL_WindowEvent>(
        m_window_resize_event_listener_id);

    Scene::OnQuit();
}

void ClientScene::registerEntity(Entity entity,
                                 const EntityInstance& instance) {
    CLIENT_CONTEXT.AttachComponentsOnEntity(entity, instance);
}

void ClientScene::initRootEntity(const Path& script_path) {
    m_root_entity = CLIENT_CONTEXT.CreateEntity();
    m_entities.insert(m_root_entity);
    CLIENT_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    CLIENT_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity,
                                                          m_root_entity);
    CLIENT_CONTEXT.m_draw_order_manager->RegisterEntity(m_root_entity);

    m_ui_root_entity = CLIENT_CONTEXT.CreateEntity();
    m_entities.insert(m_ui_root_entity);
    CLIENT_CONTEXT.m_transform_manager->RegisterEntity(m_ui_root_entity);
    CLIENT_CONTEXT.m_relationship_manager->RegisterEntity(m_ui_root_entity,
                                                          m_ui_root_entity);
    CLIENT_CONTEXT.m_ui_manager->RegisterEntity(m_ui_root_entity);
    CLIENT_CONTEXT.m_draw_order_manager->RegisterEntity(m_ui_root_entity);
    UIWidget* ui = CLIENT_CONTEXT.m_ui_manager->Get(m_ui_root_entity);
    ui->m_anchor = UIAnchor::None;
    ui->m_panel = std::make_unique<UIPanelComponent>();
    Transform* transform =
        CLIENT_CONTEXT.m_transform_manager->Get(m_ui_root_entity);
    transform->m_size = CLIENT_CONTEXT.m_window->GetWindowSize();

    if (!script_path.empty()) {
        auto handle =
            CLIENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
                .Load(script_path);
        CLIENT_CONTEXT.m_script_component_manager->RegisterEntity(
            m_root_entity, m_root_entity, handle);
    }
}

SceneHandle ClientSceneManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<ClientScene>(filename));
}

SceneHandle ClientSceneManager::Create(SceneDefinitionHandle handle) {
    return store(nullptr, UUID::CreateV4(), std::make_unique<ClientScene>(handle));
}
