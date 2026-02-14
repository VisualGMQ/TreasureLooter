#include "engine/script/script_binding.hpp"
#include "engine/script/script_handle_binding.hpp"
#include "engine/asset_manager.hpp"
#include "schema/binding/binding.hpp"
#include "engine/animation.hpp"
#include "engine/context.hpp"
#include "engine/entity.hpp"
#include "engine/handle.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/path.hpp"
#include "engine/script/script.hpp"
#include "engine/text.hpp"
#include "engine/timer.hpp"
#include "engine/math.hpp"
#include "engine/transform.hpp"
#include "engine/level.hpp"
#include "engine/image.hpp"
#include "engine/sprite.hpp"
#include "engine/tilemap.hpp"
#include "engine/renderer.hpp"
#include "engine/window.hpp"
#include "engine/camera.hpp"
#include "engine/trigger.hpp"
#include "engine/input/input.hpp"
#include "engine/relationship.hpp"
#include "engine/physics.hpp"
#include "engine/cct.hpp"
#include "engine/collision_group.hpp"
#include "engine/ui.hpp"
#include "engine/text.hpp"
#include "engine/animation_player.hpp"
#include "engine/gameplay_config.hpp"
#include "engine/debug_drawer.hpp"
#include "engine/prefab_manager.hpp"
#include "schema/prefab.hpp"
#include "imgui.h"
#include <string>
#include <type_traits>
#include <sstream>

#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Array.h"
#include "LuaBridge/Vector.h"
#include "LuaBridge/Map.h"
#include "LuaBridge/UnorderedMap.h"
#include "LuaBridge/Set.h"
#include "LuaBridge/List.h"

template <>
struct luabridge::Stack<Entity>: public luabridge::Enum<Entity> {};

static int ScriptComponentManager_Get(lua_State* L) {
    auto mgrRes = luabridge::get<ScriptComponentManager*>(L, 1);
    auto eRes = luabridge::get<Entity>(L, 2);
    if (!mgrRes || !eRes) {
        lua_pushnil(L);
        return 1;
    }
    Script* s = (*mgrRes)->Get(*eRes);
    if (!s) {
        lua_pushnil(L);
        return 1;
    }
    lua_getref(L, s->GetScriptTableRef());
    return 1;
}

static void bindScriptBinaryDataManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<ScriptBinaryDataManager>("ScriptBinaryDataManager")
        .addFunction("Load",
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
        .addFunction("Get", ScriptComponentManager_Get)
        .addFunction("Update", &ScriptComponentManager::Update)
        .addFunction("Render", &ScriptComponentManager::Render)
        .endClass()
        .endNamespace();
}

static void bindEntity(lua_State* L) {
    (void)L;
    /* Entity 在 Lua 中为栈上传递的 number（枚举底层值），无需注册类 */
}

static void bindPath(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Path>("Path")
        .addConstructor<void (const std::string&), void (void)>()
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
        .addFunction("__div",
                     +[](const Path& a, const Path& b) { return a / b; })
        .addFunction("__div",
                     +[](const Path& p, const std::string& s) { return p / s; })
        .endClass()
        .endNamespace();
}

template <typename T>
static void bindTVec2(lua_State* L, const char* className) {
    using Scalar = T;
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<TVec2<T>>(className)
        .template addConstructor<void (void), void (Scalar, Scalar)>()
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

static void bindMath(lua_State* L) {
    bindTVec2<float>(L, "Vec2");
    bindTVec2<uint32_t>(L, "Vec2UI");
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Color>("Color")
        .addConstructor<void (void), void (float, float, float, float)>()
        .addProperty("r", &Color::r, true)
        .addProperty("g", &Color::g, true)
        .addProperty("b", &Color::b, true)
        .addProperty("a", &Color::a, true)
        .endClass()
        .addFunction("Red", +[]() { return Color::Red; })
        .addFunction("Green", +[]() { return Color::Green; })
        .addFunction("Blue", +[]() { return Color::Blue; })
        .addFunction("Black", +[]() { return Color::Black; })
        .addFunction("White", +[]() { return Color::White; })
        .addFunction("Yellow", +[]() { return Color::Yellow; })
        .addFunction("Purple", +[]() { return Color::Purple; })
        .addFunction("GetZOrderByYSorting", &GetZOrderByYSorting)
        .beginClass<Degrees>("Degrees")
        .addConstructor<void (void), void (float)>()
        .addFunction("Value", &Degrees::Value)
        .addFunction("__add", &Degrees::operator+)
        .addFunction("__sub", &Degrees::operator-)
        .addFunction("__mul",
                     (Degrees(Degrees::*)(float) const)&Degrees::operator*)
        .addFunction("__div",
                     (Degrees(Degrees::*)(float) const)&Degrees::operator/)
        .endClass()
        .beginClass<Radians>("Radians")
        .addConstructor<void (void), void (float)>()
        .addFunction("Value", &Radians::Value)
        .addFunction("__add", &Radians::operator+)
        .addFunction("__sub", &Radians::operator-)
        .addFunction("__mul",
                     (Radians(Radians::*)(float) const)&Radians::operator*)
        .addFunction("__div",
                     (Radians(Radians::*)(float) const)&Radians::operator/)
        .endClass()
        .beginClass<Transform>("Transform")
        .addProperty("m_position", &Transform::m_position, true)
        .addProperty("m_rotation", &Transform::m_rotation, true)
        .addProperty("m_size", &Transform::m_size, true)
        .addProperty("m_scale", &Transform::m_scale, true)
        .endClass()
        .beginClass<Region>("Region")
        .addConstructor<void ()>()
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

static void bindLevel(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Level>("Level")
        .addFunction("Instantiate", &Level::Instantiate)
        .addFunction("RemoveEntity", &Level::RemoveEntity)
        .addFunction("GetRootEntity", &Level::GetRootEntity)
        .addFunction("GetUIRootEntity", &Level::GetUIRootEntity)
        .endClass()
        .endNamespace();
}

static void bindImage(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Image>("Image")
        .addFunction("GetSize", &Image::GetSize)
        .addFunction("ChangeColorMask", &Image::ChangeColorMask)
        .endClass()
        .beginClass<ImageManager>("ImageManager")
        .addFunction("Load", +[](ImageManager* m, const std::string& path, bool force) {
            return m->Load(Path(path), force);
        })
        .addFunction("Find", +[](ImageManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindContext(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
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
        .addFunction("GetLevelManager",
                     +[](GameContext* ctx) -> LevelManager* {
                         return ctx->m_level_manager.get();
                     })
        .addFunction("GetTime",
                     +[](GameContext* ctx) -> Time* {
                         return ctx->m_time.get();
                     })
        .addFunction("GetTransformManager",
                     +[](GameContext* ctx) -> TransformManager* {
                         return ctx->m_transform_manager.get();
                     })
        .addFunction("GetSpriteManager",
                     +[](GameContext* ctx) -> SpriteManager* {
                         return ctx->m_sprite_manager.get();
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
        .addFunction("GetUIManager",
                     +[](GameContext* ctx) -> UIComponentManager* {
                         return ctx->m_ui_manager.get();
                     })
        .addFunction("GetGameplayConfigManager",
                     +[](GameContext* ctx) -> GameplayConfigManager* {
                         return ctx->m_gameplay_config_manager.get();
                     })
        .addFunction("GetTilemapComponentManager",
                     +[](GameContext* ctx) -> TilemapComponentManager* {
                         return ctx->m_tilemap_component_manager.get();
                     })
        .addFunction("GetCCTManager",
                     +[](GameContext* ctx) -> CCTManager* {
                         return ctx->m_cct_manager.get();
                     })
        .endClass()
        .addFunction("GetContext", +[]() -> GameContext* {
            return &GameContext::GetInst();
        })
        .endNamespace();
}

static void bindAssetsManager(lua_State* L) {
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
        .addFunction("GetLevelManager",
                     +[](AssetsManager* m) -> LevelManager* {
                         return &m->GetManager<Level>();
                     })
        .addFunction("GetPrefabManager",
                     +[](AssetsManager* m) -> PrefabManager* {
                         return &m->GetManager<Prefab>();
                     })
        .addFunction("GetFontManager",
                     +[](AssetsManager* m) -> FontManager* {
                         return &m->GetManager<Font>();
                     })
        .endClass()
        .endNamespace();
}

static void bindInput(lua_State* L) {
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
        .endNamespace();
}

static void bindTimer(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Time>("Time")
        .addFunction("GetElapseTime", &Time::GetElapseTime)
        .addFunction("GetCurrentTime", &Time::GetCurrentTime)
        .endClass()
        .endNamespace();
}

static void bindCamera(lua_State* L) {
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

static void bindSprite(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<SpriteDefinition>("Sprite")
        .addProperty("m_region", &SpriteDefinition::m_region, true)
        .addProperty("m_size", &SpriteDefinition::m_size, true)
        .addProperty("m_anchor", &SpriteDefinition::m_anchor, true)
        .addProperty("m_z_order", &SpriteDefinition::m_z_order, true)
        .addProperty("m_color", &SpriteDefinition::m_color, true)
        .endClass()
        .beginClass<SpriteManager>("SpriteManager")
        .addFunction("Get", +[](SpriteManager* m, Entity e) {
            return m->Get(e);
        })
        .addFunction("Has", +[](SpriteManager* m, Entity e) {
            return m->Has(e);
        })
        .addFunction("Update", &SpriteManager::Update)
        .addFunction("RegisterEntity",
                     +[](SpriteManager* m, Entity e,
                         const SpriteDefinition& s) {
                         m->RegisterEntity(e, s);
                     })
        .endClass()
        .endNamespace();
}

static void bindAnimationPlayer(lua_State* L) {
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
        .addFunction("Update", &AnimationPlayer::Update)
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
        .addFunction("Update", &AnimationPlayerManager::Update)
        .endClass()
        .endNamespace();
}

static void bindCCT(lua_State* L) {
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

static void bindGameplayConfigManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<GameplayConfigManager>("GameplayConfigManager")
        .addFunction("Get", +[](GameplayConfigManager* m, Entity e) {
            return m->Get(e);
        })
        .addFunction("Has", +[](GameplayConfigManager* m, Entity e) {
            return m->Has(e);
        })
        .addFunction("RegisterEntity",
                     +[](GameplayConfigManager* m, Entity e,
                         const GameplayConfig& config) {
                         m->RegisterEntity(e, config);
                     })
        .endClass()
        .endNamespace();
}

static void bindRelationship(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Relationship>("Relationship")
        .addProperty("m_children", &Relationship::m_children)
        .endClass()
        .beginClass<RelationshipManager>("RelationshipManager")
        .addFunction("Get", +[](RelationshipManager* m, Entity e) {
            return m->Get(e);
        })
        .addFunction("Has", +[](RelationshipManager* m, Entity e) {
            return m->Has(e);
        })
        .addFunction("Update", &RelationshipManager::Update)
        .addFunction("RegisterEntity",
                     +[](RelationshipManager* m, Entity e) {
                         m->RegisterEntity(e);
                     })
        .endClass()
        .endNamespace();
}

static void bindPrefabManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<PrefabManager>("PrefabManager")
        .addFunction("Find", +[](PrefabManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindFontManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<FontManager>("FontManager")
        .addFunction("Load", +[](FontManager* m, const std::string& path, bool force) {
            return m->Load(Path(path), force);
        })
        .addFunction("Find", +[](FontManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindAnimationManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<AnimationManager>("AnimationManager")
        .addFunction("Load", +[](AnimationManager* m, const std::string& path, bool force) {
            return m->Load(Path(path), force);
        })
        .addFunction("Find", +[](AnimationManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindTilemapManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<TilemapManager>("TilemapManager")
        .addFunction("Load", +[](TilemapManager* m, const std::string& path, bool force) {
            return m->Load(Path(path), force);
        })
        .addFunction("Find", +[](TilemapManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindLevelManager(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<LevelManager>("LevelManager")
        .addFunction("Load", +[](LevelManager* m, const std::string& path, bool force) {
            return m->Load(Path(path), force);
        })
        .addFunction("Find", +[](LevelManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .addFunction("GetCurrentLevel", &LevelManager::GetCurrentLevel)
        .addFunction("Switch", &LevelManager::Switch)
        .endClass()
        .endNamespace();
}

static void bindHandleTypes(lua_State* L) {
    // Handle 通过 __index 转发到已注册的 T 类，无需再为每个 Handle 列举 T 的方法
    BindHandle<Image>("ImageHandle", L, "Image");
    BindHandle<Level>("LevelHandle", L, "Level");
    BindHandle<Prefab>("PrefabHandle", L, "Prefab");
    BindHandle<Animation>("AnimationHandle", L, "Animation");
}

static void bindAllTypes(lua_State* L) {
    bindScriptBinaryDataManager(L);
    bindEntity(L);
    bindPath(L);
    bindMath(L);
    bindLevel(L);
    bindImage(L);
    bindContext(L);
    bindAssetsManager(L);
    bindInput(L);
    bindTimer(L);
    bindCamera(L);
    bindSprite(L);
    bindAnimationPlayer(L);
    bindCCT(L);
    bindGameplayConfigManager(L);
    bindRelationship(L);
    bindPrefabManager(L);
    bindLevelManager(L);
    bindFontManager(L);
    bindAnimationManager(L);
    bindTilemapManager(L);
    bindHandleTypes(L);
}

void BindTLModule(lua_State* L) {
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "lua_State* is null!");

    bindAllTypes(L);
    BindSchema(L);
}
