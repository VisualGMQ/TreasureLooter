#pragma once
#include "common/math.hpp"
#include "common/entity.hpp"
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
struct GameConfig;
class ScriptComponentManager;
class Renderer;
class PhysicsScene;
class TilemapLayerCollisionComponentManager;
class SceneManager;
class StaticCollisionManager;
class EventSystem;
class EventDebugger;

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

    bool IsRunning() const;

    Entity CreateEntity();

    bool ShouldExit() const;
    bool IsInited() const;
    void Exit();
    const GameConfig& GetGameConfig() const;

    const std::vector<std::string_view>& GetOSArgs() const;
    std::string_view GetAppPath() const;

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
    std::unique_ptr<IDebugDrawer> m_debug_drawer;

protected:
    class ImGuiContext* m_imgui_context{};

    // call by child class
    void initGameConfig();

private:
    static CommonContext* m_current_context;
    std::vector<std::string_view> m_args;

    bool m_should_exit = true;
    bool m_is_inited = false;
    GameConfig m_game_config;
    std::underlying_type_t<Entity> m_last_entity = 1;
};

#define COMMON_CONTEXT ::CommonContext::GetInst()
