#include "common/context.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/debug_drawer.hpp"
#include "common/entity_name_manager.hpp"
#include "common/event.hpp"
#include "common/net/udp.hpp"
#include "common/profile.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/lua_event_listener.hpp"
#include "common/script/script.hpp"
#include "common/script/script_binding.hpp"
#include "common/sdl_call.hpp"
#include "common/serialize.hpp"
#include "common/static_collision.hpp"
#include "common/storage.hpp"
#include "common/tilemap.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "common/uuid.hpp"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include <memory>

CommonContext* CommonContext::m_current_context{};

void CommonContext::initCommonConfig() {
    auto handle = m_assets_manager->GetManager<CommonConfig>().Load(
        std::string{"assets/gpa/common_config"} +
        CommonConfig_AssetExtension.data());
    if (!handle) {
        LOGC("common config not found!");
        SDL_Quit();
        return;
    }
    m_common_config = *handle;
    m_assets_manager->GetManager<CommonConfig>().Unload(handle);
}

void CommonContext::ChangeContext(CommonContext& ctx) {
    m_current_context = &ctx;
}

CommonContext& CommonContext::GetInst() {
    return *m_current_context;
}

CommonContext::CommonContext() {}

CommonContext::~CommonContext() {}

void CommonContext::InitSystem() {
    LOGT("system init");
    SDL_CALL(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                      SDL_INIT_GAMEPAD));
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    SDL_CALL(TTF_Init());

    UDPInit();
}

void CommonContext::ShutdownSystem() {
    UDPShutdown();
    TTF_Quit();
    SDL_Quit();
    LOGT("system shutdown");
}

void CommonContext::Initialize(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        m_args.push_back(argv[i]);
    }

    m_should_exit = false;
    m_is_inited = true;

    m_transform_manager = std::make_unique<TransformManager>();
    m_relationship_manager = std::make_unique<RelationshipManager>();
    m_entity_name_manager = std::make_unique<EntityNameManager>();

    // event relative
    m_event_system = std::make_unique<EventSystem>();
    LuaEventListenerRegistry::Initialize(COMMON_CONTEXT.m_event_system.get());
    m_event_debugger_system = std::make_unique<EventDebugger>();
    m_time = std::make_unique<Time>();

    // physics related
    m_physics_scene = std::make_unique<PhysicsScene>();
    m_cct_manager = std::make_unique<CCTManager>();
    m_trigger_component_manager = std::make_unique<TriggerComponentManager>();
    m_static_collision_manager = std::make_unique<StaticCollisionManager>();

    // misc
    m_timer_manager = std::make_unique<TimerManager>();
    m_bind_point_component_manager =
        std::make_unique<BindPointsComponentManager>();
    m_tilemap_layer_collision_component_manager =
        std::make_unique<TilemapLayerCollisionComponentManager>();
    m_script_component_manager = std::make_unique<ScriptComponentManager>();
    m_replicate_component_manager =
        std::make_unique<ReplicateComponentManager>();
}

void CommonContext::Shutdown() {
    m_should_exit = true;
    m_is_inited = false;

    m_scene_manager.reset();

    m_script_component_manager.reset();
    // remove luau event binding firstly, to clear LuaRef & lua_State* depends
    // 1. remove all lua event listener(EventSystem will pending the remove
    // operations)
    // 2. call EventSystem::Update to actually remove them
    LuaEventListenerRegistry::Clear();
    m_event_system->Update();

    m_script_binary_data_manager.reset();

    m_trigger_component_manager.reset();
    m_bind_point_component_manager.reset();
    m_timer_manager.reset();
    m_cct_manager.reset();
    m_static_collision_manager.reset();
    m_physics_scene.reset();
    m_time.reset();

    m_event_debugger_system.reset();
    m_event_system.reset();

    m_tilemap_layer_collision_component_manager.reset();
    m_transform_manager.reset();
    m_assets_manager.reset();

    m_replicate_component_manager.reset();
    m_net_host.reset();
    m_entity_name_manager.reset();
}

void CommonContext::HandleEvents(const SDL_Event& event) {
    m_event_system->HandleEvent(event);
}

void CommonContext::AttachComponentsOnEntity(Entity entity,
                                             const EntityInstance& instance) {
    const Transform* transform =
        instance.m_transform ? &instance.m_transform.value() : nullptr;
    auto& prefab = *instance.m_prefab;

    m_transform_manager->RegisterEntity(
        entity, transform ? *transform : prefab.m_transform.value());
    if (prefab.m_tilemap_layer) {
        m_tilemap_layer_collision_component_manager->RegisterEntity(
            entity, TilemapLayerCollisionComponent{
                        entity, prefab.m_tilemap_layer.value(),
                        m_common_config.m_tile_in_chunk_size});
    }
    if (prefab.m_cct) {
        m_cct_manager->RegisterEntity(entity, entity, prefab.m_cct.value());
        m_cct_manager->Get(entity)->Teleport(transform->m_position);
    }
    if (prefab.m_name) {
        m_entity_name_manager->RegisterEntity(entity, prefab.m_name.value());
    }
    if (prefab.m_trigger) {
        m_trigger_component_manager->RegisterEntity(entity, entity,
                                                    prefab.m_trigger.value());
    }
    if (!prefab.m_bind_points.empty()) {
        m_bind_point_component_manager->RegisterEntity(entity,
                                                       prefab.m_bind_points);
    }
    if (prefab.m_static_collision.has_value()) {
        m_static_collision_manager->RegisterEntity(
            entity, entity, prefab.m_static_collision.value());
    }

    // every entity has relationship
    m_relationship_manager->RegisterEntity(entity, entity);
    auto relationship = m_relationship_manager->Get(entity);
    for (auto child : prefab.m_children) {
        Entity child_entity =
            m_scene_manager->GetCurrentScene()->Instantiate(child);
        relationship->AddChild(child_entity);
    }
}

void CommonContext::RemoveAllComponentsOnEntity(Entity entity) {
    m_transform_manager->RemoveEntity(entity);
    m_relationship_manager->RemoveEntity(entity);
    m_tilemap_layer_collision_component_manager->RemoveEntity(entity);
    m_cct_manager->RemoveEntity(entity);
    m_trigger_component_manager->RemoveEntity(entity);
    m_static_collision_manager->RemoveEntity(entity);
    m_bind_point_component_manager->RemoveEntity(entity);
    m_script_component_manager->RemoveEntity(entity);
}

void CommonContext::InitGlobalScript(const Path& script_path) {
    if (script_path.empty()) {
        return;
    }
    auto handle =
        m_assets_manager->GetManager<ScriptBinaryData>().Load(script_path);
    m_global_script = std::make_unique<Script>(null_entity, handle);
}

bool CommonContext::IsRunning() const {
    return !m_should_exit;
}

Entity CommonContext::CreateEntity() {
    return static_cast<Entity>(m_last_entity++);
}

const CommonConfig& CommonContext::GetCommonConfig() const {
    return m_common_config;
}

bool CommonContext::ShouldExit() const {
    return m_should_exit;
}

bool CommonContext::IsInited() const {
    return m_is_inited;
}

void CommonContext::Exit() {
    m_should_exit = true;
}

const std::vector<std::string_view>& CommonContext::GetOSArgs() const {
    return m_args;
}

std::string_view CommonContext::GetAppPath() const {
    return m_args[0];
}