#include "common/script/script_binding.hpp"
#include "common/animation.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/collision_group.hpp"
#include "common/context.hpp"
#include "common/debug_drawer.hpp"
#include "common/entity.hpp"
#include "common/event.hpp"
#include "common/handle.hpp"
#include "common/image.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/math.hpp"
#include "common/path.hpp"
#include "common/physics.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/script.hpp"
#include "common/script/script_flags_binding.hpp"
#include "common/script/script_handle_binding.hpp"
#include "common/script/script_imgui_binding.hpp"
#include "common/static_collision.hpp"
#include "common/tilemap.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/timer.hpp"
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "imgui.h"
#include "schema/binding/binding.hpp"
#include "schema/prefab.hpp"

#include <sstream>
#include <string>
#include <type_traits>

#include "common/font.hpp"
#include "common/type_index.hpp"
#include "schema/scene_definition.hpp"

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
            .beginClass<ImageBase>("Image")
                .addFunction("GetSize", &ImageBase::GetSize)
                .addFunction("ChangeColorMask", &ImageBase::ChangeColorMask)
            .endClass()
            .beginClass<ImageManagerBase>("ImageManager")
                .addFunction("Load",
                             +[](ImageManagerBase* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](ImageManagerBase* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](ImageManagerBase* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](ImageManagerBase* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](ImageManagerBase* m, const std::string& path) {
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
    luabridge::getGlobalNamespace(L).beginNamespace("TL").addFunction("Log", TL_Log).endNamespace();
}

ScriptBinaryDataManager* scriptGetScriptBinaryDataManager(IAssetsManager* m) {
    return &m->GetManager<ScriptBinaryData>();
}

ImageManagerBase* scriptGetImageManager(IAssetsManager* m) {
    return &m->GetManager<ImageHandle::underlying_type>();
}

TilemapManager* scriptGetTilemapManager(IAssetsManager* m) {
    return &m->GetManager<Tilemap>();
}

AnimationManager* scriptGetAnimationManager(IAssetsManager* m) {
    return &m->GetManager<Animation>();
}

GenericAssetManager<SceneDefinition>* scriptGetSceneDefinitionManager(IAssetsManager* m) {
    return &m->GetSceneDefinitionAssetManager();
}

FontManagerBase* scriptGetFontManager(IAssetsManager* m) {
    return &m->GetManager< FontHandle::underlying_type>();
}

GenericAssetManager<LevelDefinition>* scriptGetLevelDefinitionManager(IAssetsManager* m) {
    return &m->GetLevelDefinitionAssetManager();
}

void bindAssetsManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<IAssetsManager>("AssetsManager")
                .addFunction("GetScriptBinaryDataManager", &scriptGetScriptBinaryDataManager)
                .addFunction("GetImageManager", &scriptGetImageManager)
                .addFunction("GetTilemapManager", &scriptGetTilemapManager)
                .addFunction("GetAnimationManager", &scriptGetAnimationManager)
                .addFunction("GetSceneDefinitionManager", &scriptGetSceneDefinitionManager)
                .addFunction("GetFontManager", &scriptGetFontManager)
                .addFunction("GetLevelDefinitionManager", &scriptGetLevelDefinitionManager)
                // Keep a typo-compatible alias for existing scripts.
                .addFunction("GetLevelDefintionManager", &scriptGetLevelDefinitionManager)
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
                .addFunction("GetPhysicsShapeCount", &StaticCollision::GetPhysicsShapeCount)
                .addFunction(
                    "GetPhysicsShape",
                    +[](StaticCollision* sc, lua_Integer lua_index) -> PhysicsShape* {
                        if (lua_index < 1 ||
                            static_cast<std::size_t>(lua_index) >
                                sc->GetPhysicsShapeCount())
                            return nullptr;
                        return sc->GetPhysicsShape(
                            static_cast<std::size_t>(lua_index) - 1);
                    })
                .addFunction(
                    "GetPhysicsShapeLocalPosition",
                    +[](StaticCollision* sc, lua_Integer lua_index) -> Vec2 {
                        if (lua_index < 1 ||
                            static_cast<std::size_t>(lua_index) >
                                sc->GetPhysicsShapeCount())
                            return {};
                        return sc->GetPhysicsShapeLocalPosition(
                            static_cast<std::size_t>(lua_index) - 1);
                    })
            .endClass()
            .beginClass<StaticCollisionManager>("StaticCollisionManager")
                .addFunction("Get", +[](StaticCollisionManager* m, Entity e) {
                    return m->Get(e);
                })
                .addFunction("Has", &StaticCollisionManager::Has)
                .addFunction("Enable", &StaticCollisionManager::Enable)
                .addFunction("IsEnable", &StaticCollisionManager::IsEnable)
                .addFunction("Disable", &StaticCollisionManager::Disable)
            .endClass()
        .endNamespace();
}

void bindEvent(lua_State* L) {
     luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
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

void bindTilemapCollisionComponent(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<TilemapLayerCollisionComponent>("TilemapCollisionComponent")
                .addFunction("GetLayer", &TilemapLayerCollisionComponent::GetLayer)
                .addFunction("GetTilemap", &TilemapLayerCollisionComponent::GetTilemap)
                .addFunction("GetTilemapCollision",
                             &TilemapLayerCollisionComponent::GetTilemapCollision)
            .endClass()
            .beginClass<TilemapLayerCollisionComponentManager>(
                "TilemapCollisionComponentManager")
                .addFunction("Get",
                             +[](TilemapLayerCollisionComponentManager* m, Entity e) {
                                 return m->Get(e);
                             })
                .addFunction("Has",
                             +[](TilemapLayerCollisionComponentManager* m, Entity e) {
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
                .addFunction("RemoveFromParent", &Relationship::RemoveFromParent)
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
            .beginClass<FontManagerBase>("FontManager")
                .addFunction("Load",
                             +[](FontManagerBase* m, const std::string& path) {
                                 return m->Load(Path(path), false);
                             },
                             +[](FontManagerBase* m, const Path& path) {
                                 return m->Load(path, false);
                             },
                             +[](FontManagerBase* m, const Path& path, bool force) {
                                 return m->Load(path, force);
                             },
                             +[](FontManagerBase* m, const std::string& path, bool force) {
                                 return m->Load(Path(path), force);
                             })
                .addFunction("Find", +[](FontManagerBase* m, const std::string& path) {
                    return m->Find(Path(path));
                })
            .endClass()
        .endNamespace();
}

void bindAnimationManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
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

void bindHandleTypes(lua_State* L) {
    BindHandle<ImageBase>("ImageHandle", L, "Image");
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

void bindUUID(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<UUID>("UUID")
            .addConstructor<void(void)>()
            .addStaticFunction("CreateV4", &UUID::CreateV4)
            .addStaticFunction("CreateFromString", &UUID::CreateFromString)
            .addFunction(
                "IsValid", +[](UUID* uuid) { return static_cast<bool>(*uuid); })
            .addFunction("__eq", &UUID::operator==)
            .addFunction("__tostring", &UUID::ToString)
        .endClass()
        .endNamespace();
}

// clang-format on

void bindAllTypes(lua_State* L) {
    bindEntity(L);
    bindUUID(L);
    bindScriptBinaryDataManager(L);
    bindPath(L);
    bindMath(L);
    bindScene(L);
    bindImage(L);
    bindDebugDraw(L);
    bindContext(L);
    bindAssetsManager(L);
    bindTimer(L);
    bindCCT(L);
    bindPhysics(L);
    bindTilemapCollisionComponent(L);
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
