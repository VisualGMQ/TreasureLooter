#include "client/scene.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/ui.hpp"
#include "client/window.hpp"
#include "common/asset_manager.hpp"
#include "common/macros.hpp"
#include "common/transform.hpp"

void ClientScene::OnEnter() {
    Scene::OnEnter();

    Transform* transform =
        COMMON_CONTEXT.m_transform_manager->Get(GetUIRootEntity());
    transform->m_size =
        static_cast<Vec2>(CLIENT_CONTEXT.m_window->GetWindowSize());

    m_window_resize_event_listener_id =
        COMMON_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
            [this](EventListenerID, const SDL_WindowEvent& event) {
                if (event.type != SDL_EVENT_WINDOW_RESIZED &&
                    event.type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED &&
                    event.type != SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
                    return;
                }
                LogicEntity entity = this->GetUIRootEntity();
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

LogicEntity ClientScene::GetUIRootEntity() const {
    return m_ui_root_entity;
}

void ClientScene::registerEntity(LogicEntity entity,
                                 const EntityInstance& instance) {
    CLIENT_CONTEXT.AttachComponentsOnLogicEntity(entity, instance);
    PresentEntity present_entity = CLIENT_CONTEXT.CreatePresentEntity(entity);
    CLIENT_CONTEXT.AttachComponentsOnPresentEntity(present_entity, instance);
}

void ClientScene::initRootEntity(SceneDefinitionHandle scene_definition) {
    m_root_entity = CLIENT_CONTEXT.CreateLogicEntity();
    m_entities.insert(m_root_entity);
    CLIENT_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    CLIENT_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity,
                                                          m_root_entity);
    PresentEntity root_present_entity =
        CLIENT_CONTEXT.CreatePresentEntity(m_root_entity);
    CLIENT_CONTEXT.m_present_transform_manager->RegisterEntity(
        root_present_entity);
    CLIENT_CONTEXT.m_draw_order_manager->RegisterEntity(root_present_entity);

    m_ui_root_entity = CLIENT_CONTEXT.CreateLogicEntity();
    m_entities.insert(m_ui_root_entity);
    CLIENT_CONTEXT.m_transform_manager->RegisterEntity(m_ui_root_entity);
    CLIENT_CONTEXT.m_relationship_manager->RegisterEntity(m_ui_root_entity,
                                                          m_ui_root_entity);
    PresentEntity ui_root_present_entity =
        CLIENT_CONTEXT.CreatePresentEntity(m_ui_root_entity);
    CLIENT_CONTEXT.m_present_transform_manager->RegisterEntity(
        ui_root_present_entity);
    CLIENT_CONTEXT.m_ui_manager->RegisterEntity(ui_root_present_entity);
    CLIENT_CONTEXT.m_draw_order_manager->RegisterEntity(ui_root_present_entity);
    UIWidget* ui = CLIENT_CONTEXT.m_ui_manager->Get(m_ui_root_entity);
    ui->m_anchor = UIAnchor::None;
    ui->m_panel = std::make_unique<UIPanelComponent>();
    Transform* transform =
        CLIENT_CONTEXT.m_transform_manager->Get(m_ui_root_entity);
    transform->m_size =
        static_cast<Vec2>(CLIENT_CONTEXT.m_window->GetWindowSize());

    auto handle =
        CLIENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>().Load(
            scene_definition->m_client_script);
    if (handle) {
        CLIENT_CONTEXT.m_script_component_manager->RegisterEntity(
            m_root_entity, m_root_entity, handle);
    }
}

void ClientScene::initEntities(SceneDefinitionHandle level_content) {
    auto root_rel =
        COMMON_CONTEXT.m_relationship_manager->Get(m_root_entity);
    auto ui_root_rel =
        COMMON_CONTEXT.m_relationship_manager->Get(m_ui_root_entity);

    for (auto& instance : level_content->m_entities) {
        LogicEntity entity = Instantiate(instance.m_prefab);
        TL_CONTINUE_IF_FALSE(entity != null_entity);

        if (instance.m_transform) {
            auto* transform =
                COMMON_CONTEXT.m_transform_manager->Get(entity);
            if (transform) {
                *transform = *instance.m_transform;
            }
        }

        if (CLIENT_CONTEXT.m_ui_manager->Get(entity) && ui_root_rel) {
            ui_root_rel->AddChild(entity);
        } else if (root_rel) {
            root_rel->AddChild(entity);
        }
    }
}

SceneHandle ClientSceneManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUIDv4::CreateV4(),
                 std::make_unique<ClientScene>(filename));
}

SceneHandle ClientSceneManager::Create(SceneDefinitionHandle handle) {
    return store(nullptr, UUIDv4::CreateV4(),
                 std::make_unique<ClientScene>(handle));
}
