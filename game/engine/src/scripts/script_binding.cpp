#include "engine/script/script_binding.hpp"
#include "engine/animation.hpp"
#include "engine/animation_player.hpp"
#include "engine/asset_manager.hpp"
#include "engine/bind_point.hpp"
#include "engine/camera.hpp"
#include "engine/cct.hpp"
#include "engine/collision_group.hpp"
#include "engine/context.hpp"
#include "engine/debug_drawer.hpp"
#include "engine/draw_order.hpp"
#include "engine/entity.hpp"
#include "engine/handle.hpp"
#include "engine/image.hpp"
#include "engine/input/input.hpp"
#include "engine/input/mouse.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/math.hpp"
#include "engine/path.hpp"
#include "engine/physics.hpp"
#include "engine/relationship.hpp"
#include "engine/renderer.hpp"
#include "engine/scene.hpp"
#include "engine/script/script.hpp"
#include "engine/script/script_flags_binding.hpp"
#include "engine/script/script_handle_binding.hpp"
#include "engine/script/script_imgui_binding.hpp"
#include "engine/sprite.hpp"
#include "engine/static_collision.hpp"
#include "engine/text.hpp"
#include "engine/tilemap.hpp"
#include "engine/timer.hpp"
#include "engine/transform.hpp"
#include "engine/trigger.hpp"
#include "engine/ui.hpp"
#include "engine/window.hpp"
#include "imgui.h"
#include "schema/binding/binding.hpp"
#include "schema/prefab.hpp"

#include <sstream>
#include <string>
#include <type_traits>

#include "engine/script/script_event_registry.hpp"
#include "schema/scene_definition.hpp"

#define TL_REGISTER_SCRIPT_UI_EVENT(EventType, EventName)                \
    do {                                                                 \
        ScriptEventRegistry::Register<EventType>(EventName);             \
        CURRENT_CONTEXT.m_event_system->AddListener<EventType>(          \
            [](EventListenerID, const EventType& event) {                \
                CURRENT_CONTEXT.m_script_component_manager->HandleEvent( \
                    event, EventName);                                   \
            });                                                          \
    } while (0)

void registerLuaScriptEventBindings() {
    TL_REGISTER_SCRIPT_UI_EVENT(UIMouseHoverEvent, "UIMouseHoverEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(UIMouseDownEvent, "UIMouseDownEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(UIMouseUpEvent, "UIMouseUpEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(UIMouseClickedEvent, "UIMouseClickedEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(UICheckToggledEvent, "UICheckToggledEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(UIDragEvent, "UIDragEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(TriggerEnterEvent, "TriggerEnterEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(TriggerTouchEvent, "TriggerTouchEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(TriggerLeaveEvent, "TriggerLeaveEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(EventDebugger::DebugEvent, "DebugEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(TimerEvent, "TimerEvent");
    TL_REGISTER_SCRIPT_UI_EVENT(TimerStopEvent, "TimerStopEvent");
}

#undef TL_REGISTER_SCRIPT_UI_EVENT

int TL_Log(lua_State* L) {
    const int argc = lua_gettop(L);
    std::string msg;
    for (int i = 1; i <= argc; ++i) {
        if (i > 1) {
            msg += " ";
        }

        size_t len = 0;
        const char* s = luaL_tolstring(L, i, &len);  // pushes tostring(arg)
        if (s && len > 0) {
            msg.append(s, len);
        } else {
            msg += "nil";
        }
        lua_pop(L, 1);  // pop tostring result
    }

    lua_Debug ar{};
    const bool hasInfo = lua_getinfo(L, 1, "sln", &ar) != 0;

    std::string file = "unknown";
    std::string func = "anonymous";
    int line = 0;

    if (hasInfo) {
        const char* source = ar.source;
        if (source && *source != '\0') {
            file = source;
        } else if (ar.short_src[0] != '\0') {
            file = ar.short_src;
        }

        if (!file.empty() && file[0] == '@') {
            file.erase(0, 1);
        }

        if (ar.name && ar.name[0] != '\0') {
            func = ar.name;
        }

        line = ar.currentline > 0 ? ar.currentline : 0;
    }

    LOGI("[Luau][{}:{}: {}]: {}", file, func, line, msg);
    return 0;
}

// clang-format off

static int ScriptComponentManager_GetTable(lua_State* L) {
    auto mgr = luabridge::Stack<ScriptComponentManager*>::get(L, 1);
    if (!mgr) {
        lua_pushnil(L);
        return 1;
    }

    auto entity = luabridge::Stack<Entity>::get(L, 2);
    if (!entity) {
        lua_pushnil(L);
        return 1;
    }

    Script* script = mgr.value()->Get(entity.value());
    if (!script || script->GetScriptTableRef() == LUA_NOREF) {
        lua_pushnil(L);
        return 1;
    }

    lua_getref(L, script->GetScriptTableRef());
    return 1;
}

void bindScriptBinaryDataManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<ScriptBinaryDataManager>("ScriptBinaryDataManager")
        .addFunction("Load",
                     +[](ScriptBinaryDataManager* m, const std::string& path) {
                         return m->Load(Path(path), false);
                     },
                     +[](ScriptBinaryDataManager* m, const Path& path) {
                         return m->Load(path, false);
                     },
                     +[](ScriptBinaryDataManager* m, const Path& path,
                         bool force) { return m->Load(path, force); },
                     +[](ScriptBinaryDataManager* m, const std::string& path,
                         bool force) {
                         return m->Load(Path(path), force);
                     })
        .addFunction("Find",
                     +[](ScriptBinaryDataManager* m, const std::string& path) {
                         return m->Find(Path(path));
                     })
        .endClass()
        .beginClass<ScriptComponentManager>("ScriptComponentManager")
            .addFunction("Has", &ScriptComponentManager::Has)
            .addFunction("Get", ScriptComponentManager_GetTable)
            .addFunction("IsEnable", &ScriptComponentManager::IsEnable)
            .addFunction(
                "SubscribeEvent",
                +[](ScriptComponentManager* m, Entity entity,
                    const std::string& event_name) {
                    if (m) {
                        m->SubscribeEvent(entity, event_name);
                    }
                })
        .endClass()
        .endNamespace();
}

void bindCollisionGroup(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<CollisionGroup>("CollisionGroup")
                .template addConstructor<void ()>()
                .addFunction("Add", &CollisionGroup::Add)
                .addFunction("Remove", &CollisionGroup::Remove)
                .addFunction("Has", &CollisionGroup::Has)
                .addFunction("Clear", &CollisionGroup::Clear)
                .addFunction("CanCollision", &CollisionGroup::CanCollision)
                .addFunction("GetUnderlying", &CollisionGroup::GetUnderlying)
                .addFunction("SetUnderlying", &CollisionGroup::SetUnderlying)
            .endClass()
        .endNamespace();
}

void bindPath(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Path>("Path")
                .template addConstructor<void (const std::string&), void (void)>()
                // .addFunction("string", &Path::string)
                .addFunction("parent_path", &Path::parent_path)
                .addFunction("filename", &Path::filename)
                .addFunction("extension", &Path::extension)
                .addFunction("has_extension", &Path::has_extension)
                .addFunction("is_absolute", &Path::is_absolute)
                .addFunction("is_relative", &Path::is_relative)
                .addFunction("empty", &Path::empty)
                .addFunction("__eq", +[](const Path& a, const Path& b) {
                    return a == b;
                })
                .addFunction("__eq", +[](const Path& a, const std::string& b) {
                    return a == b;
                })
                .addFunction("__div",
                             +[](const Path& a, const Path& b) { return a / b; })
                .addFunction("__div",
                             +[](const Path& p, const std::string& s) { return p / s; })
                .addFunction("__tostring", +[](const Path& v) {
                    std::stringstream ss;
                    ss << "Path(" << v << ")";
                    return ss.str();
                })
            .endClass()
        .endNamespace();
}

template <typename T>
void bindTVec2(lua_State* L, const char* className) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<TVec2<T>>(className)
                .template addConstructor<void (void), void (T, T)>()
                .addProperty("x", &TVec2<T>::x, true)
                .addProperty("y", &TVec2<T>::y, true)
                .addFunction("__add", &TVec2<T>::operator+)
                .addFunction("__sub", &TVec2<T>::operator-)
                .addFunction("__mul",
                             (TVec2<T>(TVec2<T>::*)(const TVec2<T>&) const)&TVec2<T>::operator*)
                .addFunction("__mul", (TVec2<T>(TVec2<T>::*)(float) const)&TVec2<T>::operator*)
                .addFunction("__div",
                             (TVec2<T>(TVec2<T>::*)(const TVec2<T>&) const)&TVec2<T>::operator/)
                .addFunction("__div", (TVec2<T>(TVec2<T>::*)(float) const)&TVec2<T>::operator/)
                .addFunction("__eq", &TVec2<T>::operator==)
                .addFunction("__tostring", +[](const TVec2<T>* v) {
                    std::stringstream ss;
                    ss << *v;
                    return ss.str();
                })
                .addFunction("Length", &TVec2<T>::Length)
                .addFunction("LengthSquared", &TVec2<T>::LengthSquared)
                .addFunction("Normalize", &TVec2<T>::Normalize)
                .addFunction("Dot", &TVec2<T>::Dot)
                .addFunction("Cross", &TVec2<T>::Cross)
                .addStaticProperty("ZERO", &TVec2<T>::ZERO)
                .addStaticProperty("X_UNIT", &TVec2<T>::X_UNIT)
                .addStaticProperty("Y_UNIT", &TVec2<T>::Y_UNIT)
            .endClass()
        .endNamespace();
}

void bindMath(lua_State* L) {
    bindTVec2<float>(L, "Vec2");
    bindTVec2<uint32_t>(L, "Vec2UI");
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Color>("Color")
                .template addConstructor<void (void), void (float, float, float, float)>()
                .addProperty("r", &Color::r, true)
                .addProperty("g", &Color::g, true)
                .addProperty("b", &Color::b, true)
                .addProperty("a", &Color::a, true)
                .addStaticProperty("Red", &Color::Red)
                .addStaticProperty("Green", &Color::Green)
                .addStaticProperty("Blue", &Color::Blue)
                .addStaticProperty("Black", &Color::Black)
                .addStaticProperty("White", &Color::White)
                .addStaticProperty("Yellow", &Color::Yellow)
                .addStaticProperty("Purple", &Color::Purple)
            .endClass()
            .addFunction("GetAngle", &GetAngle)
            .addFunction("DecomposeVector", &DecomposeVector)
            .beginClass<DecompositionResult>("DecompositionResult")
                .addProperty("m_tangent", &DecompositionResult::m_tangent, true)
                .addProperty("m_normal", &DecompositionResult::m_normal, true)
            .endClass()
            .beginClass<Degrees>("Degrees")
                .template addConstructor<void (void), void (float), void(Radians)>()
                .addFunction("Value", &Degrees::Value)
                .addFunction("__add", &Degrees::operator+)
                .addFunction("__sub", &Degrees::operator-)
                .addFunction("__mul",
                             (Degrees(Degrees::*)(float) const)&Degrees::operator*)
                .addFunction("__div",
                             (Degrees(Degrees::*)(float) const)&Degrees::operator/)
                .addFunction("__tostring", +[](const Degrees* v) {
                                std::stringstream ss;
                                ss << *v;
                                return ss.str();
                            })
            .endClass()
            .beginClass<Radians>("Radians")
                .template addConstructor<void (void), void (float), void(Degrees)>()
                .addFunction("Value", &Radians::Value)
                .addFunction("__add", &Radians::operator+)
                .addFunction("__sub", &Radians::operator-)
                .addFunction("__mul",
                             (Radians(Radians::*)(float) const)&Radians::operator*)
                .addFunction("__div",
                             (Radians(Radians::*)(float) const)&Radians::operator/)
                .addFunction("__tostring", +[](const Radians* v) {
                                std::stringstream ss;
                                ss << *v;
                                return ss.str();
                            })
            .endClass()
            .beginClass<Transform>("Transform")
                .addConstructor<void(void)>()
                .addProperty("m_position", &Transform::m_position, true)
                .addProperty("m_rotation", &Transform::m_rotation, true)
                .addProperty("m_size", &Transform::m_size, true)
                .addProperty("m_scale", &Transform::m_scale, true)
                .addFunction("GetGlobalPosition",
                             +[](Transform* t) {
                                 return GetPosition(t->GetGlobalMat());
                             })
            .endClass()
            .beginClass<Region>("Region")
                .template addConstructor<void ()>()
                .addProperty("m_topleft", &Region::m_topleft, true)
                .addProperty("m_size", &Region::m_size, true)
            .endClass()
            .beginClass<Image9Grid>("Image9Grid")
                .template addConstructor<void ()>()
                .addProperty("left", &Image9Grid::left, true)
                .addProperty("right", &Image9Grid::right, true)
                .addProperty("top", &Image9Grid::top, true)
                .addProperty("bottom", &Image9Grid::bottom, true)
                .addProperty("scale", &Image9Grid::scale, true)
                .addFunction("IsValid", &Image9Grid::IsValid)
            .endClass()
            .beginClass<TransformManager>("TransformManager")
                .addFunction("Get", +[](TransformManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](TransformManager* m, Entity e) {
                    return m->Has(e);
                })
            .endClass()
        .endNamespace();
}

void bindScene(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Scene>("Scene")
                .addFunction("Instantiate", &Scene::Instantiate)
                .addFunction("RemoveEntity", &Scene::RemoveEntity)
                .addFunction("GetRootEntity", &Scene::GetRootEntity)
                .addFunction("GetUIRootEntity", &Scene::GetUIRootEntity)
            .endClass()
        .endNamespace();
}

void bindImage(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Image>("Image")
                .addFunction("GetSize", &Image::GetSize)
                .addFunction("ChangeColorMask", &Image::ChangeColorMask)
            .endClass()
            .beginClass<ImageManager>("ImageManager")
                .addFunction("Load",
                             +[](ImageManager* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](ImageManager* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](ImageManager* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](ImageManager* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](ImageManager* m, const std::string& path) {
                    return m->Find(Path(path));
                })
            .endClass()
        .endNamespace();
}

void bindDebugDraw(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<IDebugDrawer>("DebugDraw")
                .addStaticProperty("kOneFrame",
                                   +[]() { return IDebugDrawer::kOneFrame; })
                .addStaticProperty("kAlways",
                                   +[]() { return IDebugDrawer::kAlways; })
                .addFunction(
                    "DrawRect",
                    +[](IDebugDrawer* d, Vec2 center, Vec2 half_size, Color color,
                        TimeType time) {
                        if (!d) {
                            return;
                        }
                        Rect r;
                        r.m_center = center;
                        r.m_half_size = half_size;
                        d->DrawRect(r, color, time, true);
                    },
                    +[](IDebugDrawer* d, Vec2 center, Vec2 half_size, Color color,
                        TimeType time, bool use_camera) {
                        if (!d) {
                            return;
                        }
                        Rect r;
                        r.m_center = center;
                        r.m_half_size = half_size;
                        d->DrawRect(r, color, time, use_camera);
                    })
                .addFunction(
                    "FillRect",
                    +[](IDebugDrawer* d, Vec2 center, Vec2 half_size, Color color,
                        TimeType time) {
                        if (!d) {
                            return;
                        }
                        Rect r;
                        r.m_center = center;
                        r.m_half_size = half_size;
                        d->FillRect(r, color, time, true);
                    },
                    +[](IDebugDrawer* d, Vec2 center, Vec2 half_size, Color color,
                        TimeType time, bool use_camera) {
                        if (!d) {
                            return;
                        }
                        Rect r;
                        r.m_center = center;
                        r.m_half_size = half_size;
                        d->FillRect(r, color, time, use_camera);
                    })
                .addFunction(
                    "DrawCircle",
                    +[](IDebugDrawer* d, Vec2 center, float radius, Color color,
                        TimeType time) {
                        if (!d) {
                            return;
                        }
                        Circle c;
                        c.m_center = center;
                        c.m_radius = radius;
                        d->DrawCircle(c, color, time, true);
                    },
                    +[](IDebugDrawer* d, Vec2 center, float radius, Color color,
                        TimeType time, bool use_camera) {
                        if (!d) {
                            return;
                        }
                        Circle c;
                        c.m_center = center;
                        c.m_radius = radius;
                        d->DrawCircle(c, color, time, use_camera);
                    })
                .addFunction(
                    "AddLine",
                    +[](IDebugDrawer* d, Vec2 p1, Vec2 p2, Color color,
                        TimeType time) {
                        if (!d) {
                            return;
                        }
                        d->AddLine(p1, p2, color, time, true);
                    },
                    +[](IDebugDrawer* d, Vec2 p1, Vec2 p2, Color color,
                        TimeType time, bool use_camera) {
                        if (!d) {
                            return;
                        }
                        d->AddLine(p1, p2, color, time, use_camera);
                    })
                .addFunction("Clear", +[](IDebugDrawer* d) {
                    if (d) {
                        d->Clear();
                    }
                })
            .endClass()
        .endNamespace();
}

void bindContext(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .addFunction("Log", TL_Log)
            .beginClass<GameContext>("GameContext")
                .addFunction("GetCamera", +[](GameContext* ctx) { return &ctx->m_camera; })
                .addFunction("GetScriptManager",
                             +[](GameContext* ctx) -> ScriptComponentManager* {
                                 return ctx->m_script_component_manager.get();
                             })
                .addFunction("GetAssetsManager",
                             +[](GameContext* ctx) -> AssetsManager* {
                                 return ctx->m_assets_manager.get();
                             })
                .addFunction("GetSceneManager",
                             +[](GameContext* ctx) -> SceneManager* {
                                 return ctx->m_scene_manager.get();
                             })
                .addFunction("GetTime",
                             +[](GameContext* ctx) -> Time* {
                                 return ctx->m_time.get();
                             })
                .addFunction("GetTimerManager",
                             +[](GameContext* ctx) -> TimerManager* {
                                 return ctx->m_timer_manager.get();
                             })
                .addFunction("GetTransformManager",
                             +[](GameContext* ctx) -> TransformManager* {
                                 return ctx->m_transform_manager.get();
                             })
                .addFunction("GetSpriteManager",
                             +[](GameContext* ctx) -> SpriteManager* {
                                 return ctx->m_sprite_manager.get();
                             })
                .addFunction("GetDrawOrderManager",
                             +[](GameContext* ctx) -> DrawOrderManager* {
                                 return ctx->m_draw_order_manager.get();
                             })
                .addFunction("GetRenderer",
                             +[](GameContext* ctx) -> Renderer* {
                                 return ctx->m_renderer.get();
                             })
                .addFunction("GetWindow",
                             +[](GameContext* ctx) -> Window* {
                                 return ctx->m_window.get();
                             })
                .addFunction("GetInputManager",
                             +[](GameContext* ctx) -> InputManager* {
                                 return ctx->m_input_manager.get();
                             })
                .addFunction("GetTriggerComponentManager",
                             +[](GameContext* ctx) -> TriggerComponentManager* {
                                 return ctx->m_trigger_component_manager.get();
                             })
                .addFunction("GetRelationshipManager",
                             +[](GameContext* ctx) -> RelationshipManager* {
                                 return ctx->m_relationship_manager.get();
                             })
                .addFunction("GetAnimationPlayerManager",
                             +[](GameContext* ctx) -> AnimationPlayerManager* {
                                 return ctx->m_animation_player_manager.get();
                             })
                .addFunction("GetBindPointsComponentManager",
                             +[](GameContext* ctx) -> BindPointsComponentManager* {
                                 return ctx->m_bind_point_component_manager.get();
                             })
                .addFunction("GetUIManager",
                             +[](GameContext* ctx) -> UIComponentManager* {
                                 return ctx->m_ui_manager.get();
                             })
                .addFunction("GetTilemapComponentManager",
                             +[](GameContext* ctx) -> TilemapLayerComponentManager* {
                                 return ctx->m_tilemap_layer_component_manager.get();
                             })
                .addFunction("GetCCTManager",
                             +[](GameContext* ctx) -> CCTManager* {
                                 return ctx->m_cct_manager.get();
                             })
                .addFunction("GetPhysicsScene",
                             +[](GameContext* ctx) -> PhysicsScene* {
                                 return ctx->m_physics_scene.get();
                             })
                .addFunction("GetStaticCollisionManager",
                             +[](GameContext* ctx) -> StaticCollisionManager* {
                                 return ctx->m_static_collision_manager.get();
                             })
                .addFunction("GetEventDebugger",
                             +[](GameContext* ctx) -> EventDebugger* {
                                 return ctx->m_event_debugger_system.get();
                             })
                .addFunction("GetDrawOrderManager",
                             +[](GameContext* ctx) -> DrawOrderManager* {
                                 return ctx->m_draw_order_manager.get();
                             })
                .addFunction("GetDebugDraw",
                             +[](GameContext* ctx) -> IDebugDrawer* {
                                 return ctx->m_debug_drawer.get();
                             })
            .endClass()
            .addFunction("GetContext", +[]() -> GameContext* {
                return &GameContext::GetInst();
            })
        .endNamespace();
}

void bindAssetsManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<AssetsManager>("AssetsManager")
                .addFunction("GetScriptBinaryDataManager",
                             +[](AssetsManager* m) -> ScriptBinaryDataManager* {
                                 return &m->GetManager<ScriptBinaryData>();
                             })
                .addFunction("GetImageManager",
                             +[](AssetsManager* m) -> ImageManager* {
                                 return &m->GetManager<Image>();
                             })
                .addFunction("GetTilemapManager",
                             +[](AssetsManager* m) -> TilemapManager* {
                                 return &m->GetManager<Tilemap>();
                             })
                .addFunction("GetAnimationManager",
                             +[](AssetsManager* m) -> AnimationManager* {
                                 return &m->GetManager<Animation>();
                             })
                .addFunction("GetSceneDefinitionManager",
                             +[](AssetsManager* m) -> GenericAssetManager<SceneDefinition>* {
                                 return &m->GetManager<SceneDefinition>();
                             })
                .addFunction("GetFontManager",
                             +[](AssetsManager* m) -> FontManager* {
                                 return &m->GetManager<Font>();
                             })
                .addFunction("GetLevelDefinitionManager",
                             +[](AssetsManager* m)
                                 -> GenericAssetManager<LevelDefinition>* {
                                 return &m->GetManager<LevelDefinition>();
                             })
                // Keep a typo-compatible alias for existing scripts.
                .addFunction("GetLevelDefintionManager",
                             +[](AssetsManager* m)
                                 -> GenericAssetManager<LevelDefinition>* {
                                 return &m->GetManager<LevelDefinition>();
                             })
            .endClass()
        .endNamespace();
}

void bindInput(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .addFunction("ACTION_PRESSED",
                         +[]() { return static_cast<int>(Action::State::Pressed); })
            .addFunction("ACTION_PRESSING",
                         +[]() { return static_cast<int>(Action::State::Pressing); })
            .addFunction("ACTION_RELEASED",
                         +[]() { return static_cast<int>(Action::State::Released); })
            .addFunction("ACTION_RELEASING",
                         +[]() { return static_cast<int>(Action::State::Releasing); })
            .beginClass<Action>("Action")
                .addFunction("IsPressed", +[](const Action* a) { return a->IsPressed(0); },
                             +[](const Action* a, int id) { return a->IsPressed(id); })
                .addFunction("IsPressing", +[](const Action* a) { return a->IsPressing(0); },
                             +[](const Action* a, int id) { return a->IsPressing(id); })
                .addFunction("IsReleased", +[](const Action* a) { return a->IsReleased(0); },
                             +[](const Action* a, int id) { return a->IsReleased(id); })
                .addFunction("IsReleasing", +[](const Action* a) { return a->IsReleasing(0); },
                             +[](const Action* a, int id) { return a->IsReleasing(id); })
                .addFunction("IsRelease", +[](const Action* a) { return a->IsRelease(0); },
                             +[](const Action* a, int id) { return a->IsRelease(id); })
                .addFunction("IsPress", +[](const Action* a) { return a->IsPress(0); },
                             +[](const Action* a, int id) { return a->IsPress(id); })
            .endClass()
            .beginClass<Axis>("Axis")
                .addFunction("Value", +[](const Axis* a) { return a->Value(0); },
                             +[](const Axis* a, int id) { return a->Value(id); })
            .endClass()
            .beginClass<Axises>("Axises")
                .addFunction("Value", +[](const Axises* a) { return a->Value(0); },
                             +[](const Axises* a, int id) { return a->Value(id); })
            .endClass()
            .beginClass<InputManager>("InputManager")
                .addFunction("GetAxis",
                             +[](const InputManager* m, const std::string& name) {
                                 return &m->GetAxis(name);
                             })
                .addFunction("GetAction",
                             +[](const InputManager* m, const std::string& name) {
                                 return &m->GetAction(name);
                             })
                .addFunction("MakeAxises",
                             +[](const InputManager* m, const std::string& x_name,
                                 const std::string& y_name) { return m->MakeAxises(x_name, y_name); })
                .addFunction("AcceptFingerAxisEvent",
                             &InputManager::AcceptFingerAxisEvent)
                .addFunction("AcceptFingerButton",
                             +[](InputManager* m, const std::string& name, int state) {
                                 m->AcceptFingerButton(name,
                                                       static_cast<Action::State>(state));
                             })
            .endClass()
            .beginClass<MouseButton>("MouseButton")
                .addFunction("IsPressing",
                             +[](const MouseButton* b) { return b->IsPressing(); })
                .addFunction("IsReleasing",
                             +[](const MouseButton* b) { return b->IsReleasing(); })
                .addFunction("IsReleased",
                             +[](const MouseButton* b) { return b->IsReleased(); })
                .addFunction("IsPressed",
                             +[](const MouseButton* b) { return b->IsPressed(); })
                .addFunction("IsPress",
                             +[](const MouseButton* b) { return b->IsPress(); })
                .addFunction("IsRelease",
                             +[](const MouseButton* b) { return b->IsRelease(); })
                .addFunction("GetLastDownTime", &MouseButton::GetLastDownTime)
                .addFunction("GetLastUpTime", &MouseButton::GetLastUpTime)
            .endClass()
        .endNamespace();
}

void bindTimer(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Time>("Time")
                .addFunction("GetElapseTime", &Time::GetElapseTime)
                .addFunction("GetCurrentTime", &Time::GetCurrentTime)
            .endClass()
            .addProperty("null_timer_id", +[]() -> TimerID { return null_timer_id; })
            .beginClass<Timer>("Timer")
                .addFunction("SetInterval", &Timer::SetInterval)
                .addFunction("Start", &Timer::Start)
                .addFunction("Stop", &Timer::Stop)
                .addFunction("Rewind", &Timer::Rewind)
                .addFunction("SetLoop", &Timer::SetLoop)
                .addFunction("Pause", &Timer::Pause)
                .addFunction("GetInterval", &Timer::GetInterval)
                .addFunction("SetEventType", &Timer::SetEventType)
                .addFunction("GetEventType", &Timer::GetEventType)
                .addFunction("GetID", &Timer::GetID)
                .addFunction("IsRunning", &Timer::IsRunning)
            .endClass()
            .beginClass<TimerManager>("TimerManager")
                .addFunction("Create", &TimerManager::Create)
                .addFunction("Remove", &TimerManager::Remove)
                .addFunction("Find", &TimerManager::Find)
            .endClass()
        .endNamespace();
}

void bindCamera(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Camera>("Camera")
                .addFunction("GetPosition", &Camera::GetPosition)
                .addFunction("GetScale", &Camera::GetScale)
                .addFunction("MoveTo", &Camera::MoveTo)
                .addFunction("Move", &Camera::Move)
                .addFunction("ChangeScale", &Camera::ChangeScale)
            .endClass()
        .endNamespace();
}

void bindSprite(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<SpriteDefinition>("Sprite")
                .addConstructor<void(void)>()
                .addProperty("m_image", &SpriteDefinition::m_image, true)
                .addProperty("m_region", &SpriteDefinition::m_region, true)
                .addProperty("m_size", &SpriteDefinition::m_size, true)
                .addProperty("m_anchor", &SpriteDefinition::m_anchor, true)
                .addProperty("m_color", &SpriteDefinition::m_color, true)
                .addProperty("m_flip", &SpriteDefinition::m_flip, true)
            .endClass()
            .beginClass<SpriteManager>("SpriteManager")
                .addFunction("Get", +[](SpriteManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](SpriteManager* m, Entity e) {
                    return m->Has(e);
                })
                .addFunction("RegisterEntity",
                             +[](SpriteManager* m, Entity e,
                                 const SpriteDefinition& s) {
                                 m->RegisterEntity(e, s);
                             })
                .addFunction("IsEnable", &SpriteManager::IsEnable)
                .addFunction("Enable", &SpriteManager::Enable)
                .addFunction("Disable", &SpriteManager::Disable)
            .endClass()
        .endNamespace();
}

void bindDrawOrder(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<DrawOrder>("DrawOrder")
                .addProperty("m_z_order", &DrawOrder::m_z_order, true)
                .addProperty("m_enable_y_sorting", &DrawOrder::m_enable_y_sorting,
                             true)
                .addFunction("GetGlobalOrder", &DrawOrder::GetGlobalOrder)
            .endClass()
            .beginClass<DrawOrderManager>("DrawOrderManager")
                .addFunction("Get", +[](DrawOrderManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](DrawOrderManager* m, Entity e) {
                    return m->Has(e);
                })
                .addFunction("RegisterEntity",
                             +[](DrawOrderManager* m, Entity e, const DrawOrderDefinition& def) {
                                 m->RegisterEntity(e, def);
                             })
            .endClass()
        .endNamespace();
}

void bindAnimationPlayer(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<AnimationPlayer>("AnimationPlayer")
                .addFunction("Play", &AnimationPlayer::Play)
                .addFunction("Pause", &AnimationPlayer::Pause)
                .addFunction("Stop", &AnimationPlayer::Stop)
                .addFunction("Rewind", &AnimationPlayer::Rewind)
                .addFunction("SetLoop", &AnimationPlayer::SetLoop)
                .addFunction("IsPlaying", &AnimationPlayer::IsPlaying)
                .addFunction("GetLoopCount", &AnimationPlayer::GetLoopCount)
                .addFunction("GetCurTime", &AnimationPlayer::GetCurTime)
                .addFunction("GetMaxTime", &AnimationPlayer::GetMaxTime)
                .addFunction("ChangeAnimation",
                             +[](AnimationPlayer* p, AnimationHandle handle) {
                                 p->ChangeAnimation(handle);
                             })
                .addFunction("ClearAnimation", &AnimationPlayer::ClearAnimation)
                .addFunction("HasAnimation", &AnimationPlayer::HasAnimation)
                .addFunction("Sync", +[](AnimationPlayer* p, Entity e) {
                    p->Sync(e);
                })
                .addFunction("SetRate", &AnimationPlayer::SetRate)
                .addFunction("GetRate", &AnimationPlayer::GetRate)
                .addFunction("EnableAutoPlay", &AnimationPlayer::EnableAutoPlay)
                .addFunction("IsAutoPlayEnabled", &AnimationPlayer::IsAutoPlayEnabled)
            .endClass()
            .beginClass<AnimationPlayerManager>("AnimationPlayerManager")
                .addFunction("Get", +[](AnimationPlayerManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](AnimationPlayerManager* m, Entity e) {
                    return m->Has(e);
                })
                .addFunction("IsEnable", &AnimationPlayerManager::IsEnable)
                .addFunction("Enable", &AnimationPlayerManager::Enable)
                .addFunction("Disable", &AnimationPlayerManager::Disable)
                .addFunction("RegisterEntity",
                             +[](AnimationPlayerManager* m, Entity e, const AnimationPlayerDefinition& def) {
                                 m->RegisterEntity(e, def);
                             })
            .endClass()
        .endNamespace();
}

void bindCCT(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<CharacterController>("CharacterController")
                .addFunction("MoveAndSlide",
                             +[](CharacterController* cct, Vec2 dir) { cct->MoveAndSlide(dir); })
                .addFunction("GetPosition", &CharacterController::GetPosition)
                .addFunction("SetSkin", &CharacterController::SetSkin)
                .addFunction("GetSkin", &CharacterController::GetSkin)
                .addFunction("SetMinDisp", &CharacterController::SetMinDisp)
                .addFunction("GetMinDisp", &CharacterController::GetMinDisp)
                .addFunction("Teleport", &CharacterController::Teleport)
                .addFunction("GetPhysicsShape",
                             static_cast<PhysicsShape* (CharacterController::*)()>(
                                 &CharacterController::GetPhysicsShape))
            .endClass()
            .beginClass<CCTManager>("CCTManager")
                .addFunction("Get", +[](CCTManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](CCTManager* m, Entity e) {
                    return m->Has(e);
                })
            .endClass()
        .endNamespace();
}

void bindPhysics(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<PhysicsShape>("PhysicsShape")
                .addFunction("GetPosition", &PhysicsShape::GetPosition)
                .addFunction("SetCollisionLayer", &PhysicsShape::SetCollisionLayer)
                .addFunction("GetCollisionLayer", &PhysicsShape::GetCollisionLayer)
                .addFunction("SetCollisionMask", &PhysicsShape::SetCollisionMask)
                .addFunction("GetCollisionMask", &PhysicsShape::GetCollisionMask)
                .addFunction("GetType", &PhysicsShape::GetType)
                .addFunction("GetOwner", &PhysicsShape::GetOwner)
                .addFunction("MoveTo", &PhysicsShape::MoveTo)
                .addFunction("Move", &PhysicsShape::Move)
                .addFunction("GetStorageType", &PhysicsShape::GetStorageType)
                .addFunction("SetQueryEnable", &PhysicsShape::SetQueryEnable)
                .addFunction("IsQueryEnabled", &PhysicsShape::IsQueryEnabled)
            .endClass()
            .beginClass<PhysicsScene>("PhysicsScene")
                .addFunction("IsEnableDebugDraw", &PhysicsScene::IsEnableDebugDraw)
                .addFunction("ToggleDebugDraw", &PhysicsScene::ToggleDebugDraw)
                .addFunction("Overlap",
                             static_cast<uint32_t (PhysicsScene::*)(
                                 const PhysicsShape&, OverlapResult*, size_t)>(
                                 &PhysicsScene::Overlap),
                             static_cast<bool (PhysicsScene::*)(
                                 const PhysicsShape&, const PhysicsShape&) const>(
                                 &PhysicsScene::Overlap))
            .endClass()
            .beginClass<OverlapResult>("OverlapResult")
                .addProperty("m_dst_entity", &OverlapResult::m_dst_entity)
                .addProperty("m_dst_shape", &OverlapResult::m_dst_shape)
            .endClass()
            .beginClass<StaticCollision>("StaticCollision")
            .endClass()
            .beginClass<StaticCollisionManager>("StaticCollisionManager")
                .addFunction("Get", +[](StaticCollisionManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", &StaticCollisionManager::Has)
            .endClass()
        .endNamespace();
}

void bindUI(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<UIWidget>("UIWidget")
                .addProperty("m_use_clip", &UIWidget::m_use_clip, true)
                .addProperty("m_disabled", &UIWidget::m_disabled, true)
                .addProperty("m_selected", &UIWidget::m_selected, true)
                .addProperty("m_can_be_selected", &UIWidget::m_can_be_selected, true)
                .addProperty("m_margin", &UIWidget::m_margin, true)
                .addProperty("m_padding", &UIWidget::m_padding, true)
            .endClass()
            .beginClass<UIComponentManager>("UIComponentManager")
                .addFunction("Get", +[](UIComponentManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](UIComponentManager* m, Entity e) {
                    return m->Has(e);
                })
            .endClass()
        .endNamespace();
}

void bindEvent(lua_State* L) {
     luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            // events
            .beginClass<UIMouseHoverEvent>("UIMouseHoverEvent")
                .addProperty("m_entity", &UIMouseHoverEvent::m_entity, true)
            .endClass()
            .beginClass<UIMouseDownEvent>("UIMouseDownEvent")
                .addProperty("m_entity", &UIMouseDownEvent::m_entity, true)
                .addFunction("GetButton",
                             +[](const UIMouseDownEvent* e) -> const MouseButton* {
                                 return static_cast<const MouseButton*>(&e->m_button);
                             })
            .endClass()
            .beginClass<UIMouseUpEvent>("UIMouseUpEvent")
                .addProperty("m_entity", &UIMouseUpEvent::m_entity, true)
                .addFunction("GetButton",
                             +[](const UIMouseUpEvent* e) -> const MouseButton* {
                                 return static_cast<const MouseButton*>(&e->m_button);
                             })
            .endClass()
            .beginClass<UIMouseClickedEvent>("UIMouseClickedEvent")
                .addProperty("m_entity", &UIMouseClickedEvent::m_entity, true)
            .endClass()
            .beginClass<UICheckToggledEvent>("UICheckToggledEvent")
                .addProperty("m_entity", &UICheckToggledEvent::m_entity, true)
                .addProperty("m_checked", &UICheckToggledEvent::m_checked, true)
            .endClass()
            .beginClass<UIDragEvent>("UIDragEvent")
                .addProperty("m_entity", &UIDragEvent::m_entity, true)
            .endClass()

            // event debugger
            .beginClass<EventDebugger>("EventDebugger")
                .addFunction("SendDebugEvent", &EventDebugger::SendDebugEvent)
                .addFunction("GetTriggeredCount", &EventDebugger::GetTriggeredCount)
            .endClass()
            .beginClass<EventDebugger::DebugEvent>("DebugEvent")
                .addConstructor<void(void)>()
                .addProperty("m_value", &EventDebugger::DebugEvent::m_value)
            .endClass()
            .beginClass<TriggerEnterEvent>("TriggerEnterEvent")
                .addFunction("GetType", &TriggerEnterEvent::GetType)
                .addFunction("GetOverlapResult", &TriggerEnterEvent::GetOverlapResult)
                .addFunction("GetSrcEntity", &TriggerEnterEvent::GetSrcEntity)
            .endClass()
            .beginClass<TriggerTouchEvent>("TriggerTouchEvent")
                .addFunction("GetType", &TriggerTouchEvent::GetType)
                .addFunction("GetOverlapResult", &TriggerTouchEvent::GetOverlapResult)
                .addFunction("GetSrcEntity", &TriggerTouchEvent::GetSrcEntity)
            .endClass()
            .beginClass<TriggerLeaveEvent>("TriggerLeaveEvent")
                .addFunction("GetType", &TriggerLeaveEvent::GetType)
                .addFunction("GetOverlapResult", &TriggerLeaveEvent::GetOverlapResult)
                .addFunction("GetSrcEntity", &TriggerLeaveEvent::GetSrcEntity)
            .endClass()
            .beginClass<TimerEvent>("TimerEvent")
                .addFunction("GetID", &TimerEvent::GetID)
                .addFunction("GetEventType", &TimerEvent::GetEventType)
            .endClass()
            .beginClass<TimerStopEvent>("TimerStopEvent")
                .addFunction("GetID", &TimerStopEvent::GetID)
                .addFunction("GetEventType", &TimerStopEvent::GetEventType)
            .endClass()
    .endNamespace();
}

void bindTilemap(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginNamespace("TilemapLayerType")
                .addProperty("Tiled",
                             +[]() {
                                 return static_cast<int>(TilemapLayer::Type::Tiled);
                             })
                .addProperty("Object",
                             +[]() {
                                 return static_cast<int>(TilemapLayer::Type::Object);
                             })
                .addProperty("Image",
                             +[]() {
                                 return static_cast<int>(TilemapLayer::Type::Image);
                             })
            .endNamespace()
            .beginNamespace("TilemapObjectType")
                .addProperty("None",
                             +[]() {
                                 return static_cast<int>(TilemapObject::Type::None);
                             })
                .addProperty("Point",
                             +[]() {
                                 return static_cast<int>(TilemapObject::Type::Point);
                             })
                .addProperty("Circle",
                             +[]() {
                                 return static_cast<int>(TilemapObject::Type::Circle);
                             })
                .addProperty("Rect",
                             +[]() {
                                 return static_cast<int>(TilemapObject::Type::Rect);
                             })
                .addProperty("Polygon",
                             +[]() {
                                 return static_cast<int>(TilemapObject::Type::Polygon);
                             })
            .endNamespace()
            .beginClass<Tile>("Tile")
                .addProperty("m_image", &Tile::m_image, true)
                .addProperty("m_region", &Tile::m_region, true)
                .addProperty("m_id", &Tile::m_id, true)
                .addProperty("m_tile_size", &Tile::m_tile_size, true)
            .endClass()
            .beginClass<TilemapTileLayer::Tile>("TilemapLayerTile")
                .addProperty("m_gid", &TilemapTileLayer::Tile::m_gid, true)
                .addFunction(
                    "GetFlipValue",
                    +[](const TilemapTileLayer::Tile* t) { return t->m_flip.Value(); })
            .endClass()
            .beginClass<Tileset>("Tileset")
                .addFunction("GetTile",
                             +[](const Tileset* ts, uint32_t gid) -> const Tile* {
                                 if (!ts->HasTile(gid)) {
                                     return nullptr;
                                 }
                                 return &ts->GetTile(gid);
                             })
                .addFunction("HasTile", &Tileset::HasTile)
                .addFunction("GetTileSize", &Tileset::GetTileSize)
            .endClass()
            .beginClass<Tilemap>("Tilemap")
                .addFunction("GetLayerCount",
                             +[](const Tilemap* m) { return m->GetLayers().size(); })
                .addFunction("GetLayer",
                             +[](const Tilemap* m, size_t index) -> const TilemapLayer* {
                                 const auto& layers = m->GetLayers();
                                 if (index >= layers.size()) {
                                     return nullptr;
                                 }
                                 return layers[index].get();
                             })
                .addFunction("GetTilesetCount",
                             +[](const Tilemap* m) { return m->GetTileset().size(); })
                .addFunction("GetTileset",
                             +[](const Tilemap* m, size_t index) -> const Tileset* {
                                 const auto& tilesets = m->GetTileset();
                                 if (index >= tilesets.size()) {
                                     return nullptr;
                                 }
                                 return &tilesets[index];
                             })
                .addFunction("GetTile", &Tilemap::GetTile)
                .addFunction("GetTileSize", &Tilemap::GetTileSize)
                .addFunction("GetFilename",
                             +[](const Tilemap* m) { return m->GetFilename().string(); })
            .endClass()
            .beginClass<TilemapLayer>("TilemapLayer")
                .addFunction("GetType", &TilemapLayer::GetType)
                .addFunction("AsTiledLayer", &TilemapLayer::AsTiledLayer)
                .addFunction("AsImageLayer", &TilemapLayer::AsImageLayer)
                .addFunction("AsObjectLayer", &TilemapLayer::AsObjectLayer)
                .addFunction("GetName",
                             +[](const TilemapLayer* l) {
                                 return std::string(l->GetName());
                             })
            .endClass()
            .deriveClass<TilemapTileLayer, TilemapLayer>("TilemapTileLayer")
                .addFunction(
                    "GetTile",
                    +[](const TilemapTileLayer* layer, int x, int y)
                        -> const TilemapTileLayer::Tile* {
                        const Vec2& sz = layer->GetSize();
                        if (x < 0 || y < 0 || static_cast<float>(x) >= sz.x ||
                            static_cast<float>(y) >= sz.y) {
                            return nullptr;
                        }
                        return &layer->GetTile(x, y);
                    })
                .addFunction("GetSize", &TilemapTileLayer::GetSize)
            .endClass()
            .deriveClass<TilemapObjectLayer, TilemapLayer>("TilemapObjectLayer")
                .addFunction("GetObjectCount",
                             +[](const TilemapObjectLayer* l) {
                                 return l->GetObjects().size();
                             })
                .addFunction("GetObject",
                             +[](const TilemapObjectLayer* l, size_t index)
                                 -> const TilemapObject* {
                                 const auto& objs = l->GetObjects();
                                 if (index >= objs.size()) {
                                     return nullptr;
                                 }
                                 return &objs[index];
                             })
            .endClass()
            .deriveClass<TilemapImageLayer, TilemapLayer>("TilemapImageLayer")
                .addFunction("GetImage", &TilemapImageLayer::GetImage)
                .addFunction("GetPosition", &TilemapImageLayer::GetPosition)
            .endClass()
            .beginClass<TilemapObject>("TilemapObject")
                .addFunction("GetType", &TilemapObject::GetType)
                .addFunction("GetName", &TilemapObject::GetName)
                .addFunction("IsVisiable", &TilemapObject::IsVisiable)
                .addFunction("AsCircle",
                             +[](const TilemapObject* o) { return o->AsCircle(); })
                .addFunction("AsRect",
                             +[](const TilemapObject* o) { return o->AsRect(); })
                .addFunction("AsPolygon",
                             +[](const TilemapObject* o) { return o->AsPolygon(); })
                .addFunction("AsPoint",
                             +[](const TilemapObject* o) { return o->AsPoint(); })
            .endClass()
            .beginClass<PhysicsScene::TilemapCollision>("TilemapCollision")
                .addProperty("m_topleft", &PhysicsScene::TilemapCollision::m_topleft,
                             true)
            .endClass()
        .endNamespace();
}

void bindTilemapComponent(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<TilemapLayerComponent>("TilemapComponent")
                .addFunction("GetLayer", &TilemapLayerComponent::GetLayer)
                .addFunction("GetTilemap", &TilemapLayerComponent::GetTilemap)
                .addFunction("GetTilemapCollision",
                             &TilemapLayerComponent::GetTilemapCollision)
            .endClass()
            .beginClass<TilemapLayerComponentManager>("TilemapComponentManager")
                .addFunction("Get", +[](TilemapLayerComponentManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](TilemapLayerComponentManager* m, Entity e) {
                    return m->Has(e);
                })
            .endClass()
        .endNamespace();
}

void bindTrigger(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Trigger>("Trigger")
                .addFunction("GetEventType",
                            &Trigger::GetEventType)
                .addFunction("SetEventType", &Trigger::SetEventType)
                .addFunction("EnableTriggerEveryFrameWhenTouch", &Trigger::EnableTriggerEveryFrameWhenTouch)
                .addFunction("IsTriggerEveryFrameWhenTouch", &Trigger::IsTriggerEveryFrameWhenTouch)
                .addFunction("GetTouchingShapes", &Trigger::GetTouchingShapes)
                .addFunction("GetUnderlyingShapes", &Trigger::GetUnderlyingShapes)
            .endClass()
            .beginClass<TriggerComponentManager>("TriggerComponentManager")
                .addFunction("Get", static_cast<Trigger*(TriggerComponentManager::*)(Entity)>(&TriggerComponentManager::Get))
                .addFunction("Has", &TriggerComponentManager::Has)
                .addFunction("Enable", &TriggerComponentManager::Enable)
                .addFunction("Disable", &TriggerComponentManager::Disable)
                .addFunction("IsEnable", &TriggerComponentManager::IsEnable)
                .addFunction("RegisterEntity", +[](TriggerComponentManager* manager, Entity entity,  const TriggerDefinition& definition) {
                        TL_RETURN_IF_NULL(manager);
                        manager->RegisterEntity(entity, entity, definition);
                        })
            .endClass()
        .endNamespace();
}

void bindRelationship(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<Relationship>("Relationship")
                .addFunction("AddChild", &Relationship::AddChild)
                .addFunction("Get", &Relationship::Get)
                .addFunction("GetChildrenCount", &Relationship::GetChildrenCount)
                .addFunction("GetParent", &Relationship::GetParent)
                .addFunction("RemoveChild", &Relationship::RemoveChild)
            .endClass()
            .beginClass<RelationshipManager>("RelationshipManager")
                .addFunction("Get", +[](RelationshipManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", +[](RelationshipManager* m, Entity e) {
                    return m->Has(e);
                })
                .addFunction("RegisterEntity",
                             +[](RelationshipManager* m, Entity e) {
                                 m->RegisterEntity(e, e);
                             })
            .endClass()
        .endNamespace();
}

void bindFontManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<FontManager>("FontManager")
                .addFunction("Load",
                             +[](FontManager* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](FontManager* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](FontManager* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](FontManager* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](FontManager* m, const std::string& path) {
                    return m->Find(Path(path));
                })
            .endClass()
        .endNamespace();
}

void bindAnimationManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .addFunction("LoadAssetAnimation", +[](const Path& path) {
                return LoadAsset<Animation>(path).m_payload;
            })
            .addFunction("SaveAssetAnimation", +[](const AnimationHandle& handle, const Path& path) {
                if (!handle) {
                    return;
                }
                SaveAsset(handle.GetUUID(), *handle, path);
            })
            .beginClass<AnimationManager>("AnimationManager")
                .addFunction("Load",
                             +[](AnimationManager* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](AnimationManager* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](AnimationManager* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](AnimationManager* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](AnimationManager* m, const std::string& path) {
                    return m->Find(Path(path));
                })
            .endClass()
        .endNamespace();
}

void bindTilemapManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<TilemapManager>("TilemapManager")
                .addFunction("Load",
                             +[](TilemapManager* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](TilemapManager* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](TilemapManager* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](TilemapManager* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](TilemapManager* m, const std::string& path) {
                    return m->Find(Path(path));
                })
            .endClass()
        .endNamespace();
}

void bindBindPoint(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<BindPoint>("BindPoint")
                .addProperty("m_position", &BindPoint::m_position)
                .addProperty("m_name", &BindPoint::m_name)
                .addFunction("GetGlobalPosition", &BindPoint::GetGlobalPosition)
            .endClass()
            .beginClass<BindPoints>("BindPoints")
                .addConstructor<void(void)>()
                .addProperty("m_bind_points", &BindPoints::m_bind_points)
            .endClass()
            .beginClass<BindPointsComponentManager>("BindPointsComponentManager")
                    .addFunction("Get", +[](BindPointsComponentManager* m, Entity e) {
                        return m->Get(e);
                    })
                    .addFunction("Has", &BindPointsComponentManager::Has)
                    .addFunction("ToggleDebugDraw", &BindPointsComponentManager::ToggleDebugDraw)
            .endClass()
    .endNamespace();
}

void bindSceneManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<SceneManager>("SceneManager")
                .addFunction("Load",
                             +[](SceneManager* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](SceneManager* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](SceneManager* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](SceneManager* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](SceneManager* m, const std::string& path) {
                    return m->Find(Path(path));
                })
                .addFunction("GetCurrentScene", &SceneManager::GetCurrentScene)
                .addFunction("Switch", &SceneManager::Switch)
                .addFunction("Create", static_cast<SceneHandle (SceneManager::*)(SceneDefinitionHandle)>(&SceneManager::Create))
                .addFunction("Unload", &SceneManager::Unload)
            .endClass()
        .endNamespace();
}

// clang-format on

void bindHandleTypes(lua_State* L) {
    BindHandle<Image>("ImageHandle", L, "Image");
    BindHandle<Scene>("SceneHandle", L, "Scene");
    BindHandle<Prefab>("PrefabHandle", L, "Prefab");
    BindHandle<Animation>("AnimationHandle", L, "Animation");
    BindHandle<Tilemap>("TilemapHandle", L, "Tilemap");
}

void bindEntity(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .addProperty(
            "null_entity",
            +[]() -> Entity { return static_cast<Entity>(null_entity); })
        .endNamespace();
}

void bindAllTypes(lua_State* L) {
    registerLuaScriptEventBindings();
    bindEntity(L);
    bindScriptBinaryDataManager(L);
    bindPath(L);
    bindMath(L);
    bindScene(L);
    bindImage(L);
    bindDebugDraw(L);
    bindContext(L);
    bindAssetsManager(L);
    bindInput(L);
    bindTimer(L);
    bindCamera(L);
    bindSprite(L);
    bindDrawOrder(L);
    bindAnimationPlayer(L);
    bindCCT(L);
    bindPhysics(L);
    bindUI(L);
    bindTilemapComponent(L);
    bindTrigger(L);
    bindRelationship(L);
    bindSceneManager(L);
    bindFontManager(L);
    bindAnimationManager(L);
    bindTilemapManager(L);
    bindTilemap(L);
    bindHandleTypes(L);
    bindCollisionGroup(L);
    bindBindPoint(L);
    bindEvent(L);
}

void BindTLModule(lua_State* L) {
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "lua_State* is null!");

    bindAllTypes(L);
    BindSchema(L);
    bindImGui(L);
}
