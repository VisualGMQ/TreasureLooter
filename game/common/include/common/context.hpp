#pragma once
#include "common/entity.hpp"
#include "common/math.hpp"
#include "common/path.hpp"
#include "net/sync.hpp"
#include "schema/config.hpp"
#include <memory>

class IDebugDrawer;
class ScriptBinaryDataManager;
class TriggerComponentManager;
class BindPointsComponentManager;
class CCTManager;
struct EntityInstance;
class RelationshipManager;
class TransformManager;
class IAssetsManager;
class Scene;
class ScriptComponentManager;
class Script;
class Renderer;
class PhysicsScene;
class TilemapLayerCollisionComponentManager;
class SceneManager;
class StaticCollisionManager;
class EventSystem;
class EventDebugger;
class UDPHost;
class EntityNameManager;
class ReplicateComponentManager;

class CommonContext {
public:
    static void ChangeContext(CommonContext&);

    static CommonContext& GetInst();

    CommonContext();
    virtual ~CommonContext();

    /**
     * initialize 3rdlib systems
     */
    virtual void InitSystem();

    /**
     * shutdown 3rdlib systems
     */
    virtual void ShutdownSystem();

    /**
     * initialize context
     */
    virtual void Initialize(int argc, char** argv);

    /**
     * shutdown context
     */
    virtual void Shutdown();

    virtual void Update() = 0;
    virtual void HandleEvents(const SDL_Event&);

    virtual void AttachComponentsOnEntity(Entity, const EntityInstance&);
    virtual void RemoveAllComponentsOnEntity(Entity);

    // Create the global (autoload-style) script from a path. No-op if empty.
    void InitGlobalScript(const Path& script_path);

    bool IsRunning() const;

    Entity CreateEntity();

    const CommonConfig& GetCommonConfig() const;

    [[nodiscard]] bool ShouldExit() const;
    [[nodiscard]] bool IsInited() const;
    void Exit();
    [[nodiscard]] const std::vector<std::string_view>& GetOSArgs() const;
    [[nodiscard]] std::string_view GetAppPath() const;

    std::unique_ptr<EventSystem> m_event_system;
    std::unique_ptr<EventDebugger> m_event_debugger_system;
    std::unique_ptr<IAssetsManager> m_assets_manager;
    std::unique_ptr<TransformManager> m_transform_manager;
    std::unique_ptr<RelationshipManager> m_relationship_manager;
    std::unique_ptr<Time> m_time;
    std::unique_ptr<PhysicsScene> m_physics_scene;
    std::unique_ptr<CCTManager> m_cct_manager;
    std::unique_ptr<SceneManager> m_scene_manager;
    std::unique_ptr<TimerManager> m_timer_manager;
    std::unique_ptr<BindPointsComponentManager> m_bind_point_component_manager;
    std::unique_ptr<TriggerComponentManager> m_trigger_component_manager;
    std::unique_ptr<StaticCollisionManager> m_static_collision_manager;
    std::unique_ptr<TilemapLayerCollisionComponentManager>
        m_tilemap_layer_collision_component_manager;
    std::unique_ptr<ScriptBinaryDataManager> m_script_binary_data_manager;
    std::unique_ptr<ScriptComponentManager> m_script_component_manager;
    // Global script: created at startup, not attached to any entity, ticked at
    // the start of every frame and kept alive across scene switches.
    std::unique_ptr<Script> m_global_script;
    std::unique_ptr<IDebugDrawer> m_debug_drawer;
    std::unique_ptr<EntityNameManager> m_entity_name_manager;
    std::unique_ptr<ReplicateComponentManager> m_replicate_component_manager;
    std::unique_ptr<UDPHost> m_net_host;

protected:
    class ImGuiContext* m_imgui_context{};

    void initCommonConfig();

private:
    static CommonContext* m_current_context;
    std::vector<std::string_view> m_args;

    bool m_should_exit = true;
    bool m_is_inited = false;
    std::underlying_type_t<Entity> m_last_entity = 1;

    CommonConfig m_common_config;
};

#define COMMON_CONTEXT ::CommonContext::GetInst()
