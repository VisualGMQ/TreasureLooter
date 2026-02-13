#include "engine/script/script_binding.hpp"
#include "engine/asset_manager.hpp"
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

#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"

using namespace luabridge;

template <>
struct luabridge::Stack<Entity> {
    static Result push(lua_State* L, Entity e) {
        lua_pushinteger(
            L, static_cast<lua_Integer>(static_cast<std::underlying_type_t<
                Entity>>(e)));
        return {};
    }

    static TypeResult<Entity> get(lua_State* L, int index) {
        if (lua_type(L, index) != LUA_TNUMBER)
            return makeErrorCode(ErrorCode::InvalidTypeCast);
        return static_cast<Entity>(static_cast<std::underlying_type_t<Entity>>(
            lua_tointeger(L, index)));
    }

    static bool isInstance(lua_State* L, int index) {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};

// -----------------------------------------------------------------------------
// Handle<T> 按值绑定：用 userdata 存一份拷贝，registry 里放 metatable
// -----------------------------------------------------------------------------
namespace {
const char kHandleKey_ScriptBinaryDataHandle[] = "TL::ScriptBinaryDataHandle";
const char kHandleKey_ImageHandle[] = "TL::ImageHandle";
const char kHandleKey_LevelHandle[] = "TL::LevelHandle";
const char kHandleKey_PrefabHandle[] = "TL::PrefabHandle";
const char kHandleKey_FontHandle[] = "TL::FontHandle";
const char kHandleKey_AnimationHandle[] = "TL::AnimationHandle";
const char kHandleKey_TilemapHandle[] = "TL::TilemapHandle";

template <typename T>
static int handleValidCFunction(lua_State* L) {
    auto* h = static_cast<Handle<T>*>(lua_touserdata(L, 1));
    lua_pushboolean(L, h && static_cast<bool>(*h));
    return 1;
}

template <typename T>
void ensureHandleMetatable(lua_State* L, const void* typeKey) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, const_cast<void*>(typeKey));
    if (!lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    lua_pop(L, 1);
    lua_newtable(L);
    lua_newtable(L);
    lua_pushcfunction(L, handleValidCFunction<T>, "valid");
    lua_setfield(L, -2, "valid");
    lua_setfield(L, -2, "__index");
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, const_cast<void*>(typeKey));
    lua_pop(L, 1);
}

template <typename T>
Result pushHandle(lua_State* L, Handle<T> h, const void* typeKey) {
    ensureHandleMetatable<T>(L, typeKey);
    void* ud = lua_newuserdata(L, sizeof(Handle<T>));
    if (!ud)
        return makeErrorCode(ErrorCode::LuaStackOverflow);
    new (ud) Handle<T>(std::move(h));
    lua_rawgetp(L, LUA_REGISTRYINDEX, const_cast<void*>(typeKey));
    lua_setmetatable(L, -2);
    return {};
}

template <typename T>
TypeResult<Handle<T>> getHandle(lua_State* L, int index, const void* typeKey) {
    if (!lua_isuserdata(L, index))
        return makeErrorCode(ErrorCode::InvalidTypeCast);
    lua_rawgetp(L, LUA_REGISTRYINDEX, const_cast<void*>(typeKey));
    if (!lua_getmetatable(L, index))
        return makeErrorCode(ErrorCode::InvalidTypeCast);
    bool same = lua_rawequal(L, -1, -2) != 0;
    lua_pop(L, 2);
    if (!same)
        return makeErrorCode(ErrorCode::InvalidTypeCast);
    void* ud = lua_touserdata(L, index);
    return *static_cast<Handle<T>*>(ud);
}

template <typename T>
bool isInstanceHandle(lua_State* L, int index, const void* typeKey) {
    if (!lua_isuserdata(L, index))
        return false;
    lua_rawgetp(L, LUA_REGISTRYINDEX, const_cast<void*>(typeKey));
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    bool hasMt = lua_getmetatable(L, index) != 0;
    bool same = hasMt && lua_rawequal(L, -1, -2) != 0;
    lua_pop(L, 2);
    return same;
}
}  // namespace

#define DEFINE_HANDLE_STACK(T, key)                                    \
    namespace luabridge {                                              \
    template <>                                                         \
    struct Stack<Handle<T>> {                                           \
        static Result push(lua_State* L, Handle<T> h) {                 \
            return pushHandle(L, std::move(h), (const void*)(key));     \
        }                                                               \
        static TypeResult<Handle<T>> get(lua_State* L, int index) {    \
            return getHandle<T>(L, index, (const void*)(key));          \
        }                                                               \
        static bool isInstance(lua_State* L, int index) {              \
            return isInstanceHandle<T>(L, index, (const void*)(key));   \
        }                                                               \
    };                                                                  \
    }

DEFINE_HANDLE_STACK(ScriptBinaryData, kHandleKey_ScriptBinaryDataHandle)
DEFINE_HANDLE_STACK(Image, kHandleKey_ImageHandle)
DEFINE_HANDLE_STACK(Level, kHandleKey_LevelHandle)
DEFINE_HANDLE_STACK(Prefab, kHandleKey_PrefabHandle)
DEFINE_HANDLE_STACK(Font, kHandleKey_FontHandle)
DEFINE_HANDLE_STACK(Animation, kHandleKey_AnimationHandle)
DEFINE_HANDLE_STACK(Tilemap, kHandleKey_TilemapHandle)

static void LuauLog(const std::string& msg) {
    LOGI("[Script]: {}", msg);
}

static void LuauLogInt(int v) {
    LOGI("[Script]: {}", v);
}

static void LuauLogFloat(double v) {
    LOGI("[Script]: {}", v);
}

static int ScriptComponentManager_Get(lua_State* L) {
    auto mgrRes = get<ScriptComponentManager*>(L, 1);
    auto eRes = get<Entity>(L, 2);
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
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Entity>("Entity")
        .addConstructor<void (*)(void)>()
        .addFunction("IsNull",
                     +[](Entity e) {
                         return e == null_entity;
                     })
        .endClass()
        .endNamespace();
}

static void bindPath(lua_State* L) {
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Path>("Path")
        .addConstructor<void (*)(const std::string&), void (*)(void)>()
        .addFunction("string", &Path::string)
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

static void bindMath(lua_State* L) {
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Vec2>("Vec2")
        .addConstructor<void (*)(void), void (*)(float, float)>()
        .addProperty("x", &Vec2::x, true)
        .addProperty("y", &Vec2::y, true)
        .addFunction("__add", &Vec2::operator+)
        .addFunction("__sub", &Vec2::operator-)
        .addFunction("__mul",
                     (Vec2(Vec2::*)(const Vec2&) const)&Vec2::operator*)
        .addFunction("__mul", (Vec2(Vec2::*)(float) const)&Vec2::operator*)
        .addFunction("__div",
                     (Vec2(Vec2::*)(const Vec2&) const)&Vec2::operator/)
        .addFunction("__div", (Vec2(Vec2::*)(float) const)&Vec2::operator/)
        .addFunction("__eq", &Vec2::operator==)
        .addFunction("Length", &Vec2::Length)
        .addFunction("LengthSquared", &Vec2::LengthSquared)
        .addFunction("Normalize", &Vec2::Normalize)
        .addFunction("Dot", &Vec2::Dot)
        .addFunction("Cross", &Vec2::Cross)
        .endClass()
        .addFunction("Vec2_ZERO", +[]() { return TVec2<float>::ZERO; })
        .addFunction("Vec2_X_UNIT", +[]() { return TVec2<float>::X_UNIT; })
        .addFunction("Vec2_Y_UNIT", +[]() { return TVec2<float>::Y_UNIT; })
        .beginClass<Color>("Color")
        .addConstructor<void (*)(void), void (*)(float, float, float, float)>()
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
        .beginClass<Degrees>("Degrees")
        .addConstructor<void (*)(void), void (*)(float)>()
        .addFunction("Value", &Degrees::Value)
        .addFunction("__add", &Degrees::operator+)
        .addFunction("__sub", &Degrees::operator-)
        .addFunction("__mul",
                     (Degrees(Degrees::*)(float) const)&Degrees::operator*)
        .addFunction("__div",
                     (Degrees(Degrees::*)(float) const)&Degrees::operator/)
        .endClass()
        .beginClass<Radians>("Radians")
        .addConstructor<void (*)(void), void (*)(float)>()
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
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<CommonContext>("CommonContext")
        .addProperty("m_camera", &CommonContext::m_camera, true)
        .endClass()
        .deriveClass<GameContext, CommonContext>("GameContext")
        .addFunction("m_script_manager",
                     +[](GameContext* ctx) -> ScriptComponentManager* {
                         return ctx->m_script_component_manager.get();
                     })
        .addFunction("m_assets_manager",
                     +[](GameContext* ctx) -> AssetsManager* {
                         return ctx->m_assets_manager.get();
                     })
        .addFunction("m_level_manager",
                     +[](GameContext* ctx) -> LevelManager* {
                         return ctx->m_level_manager.get();
                     })
        .addFunction("m_time",
                     +[](GameContext* ctx) -> Time* {
                         return ctx->m_time.get();
                     })
        .addFunction("m_transform_manager",
                     +[](GameContext* ctx) -> TransformManager* {
                         return ctx->m_transform_manager.get();
                     })
        .addFunction("m_sprite_manager",
                     +[](GameContext* ctx) -> SpriteManager* {
                         return ctx->m_sprite_manager.get();
                     })
        .addFunction("m_renderer",
                     +[](GameContext* ctx) -> Renderer* {
                         return ctx->m_renderer.get();
                     })
        .addFunction("m_window",
                     +[](GameContext* ctx) -> Window* {
                         return ctx->m_window.get();
                     })
        .addFunction("m_input_manager",
                     +[](GameContext* ctx) -> InputManager* {
                         return ctx->m_input_manager.get();
                     })
        .addFunction("m_trigger_component_manager",
                     +[](GameContext* ctx) -> TriggerComponentManager* {
                         return ctx->m_trigger_component_manager.get();
                     })
        .addFunction("m_relationship_manager",
                     +[](GameContext* ctx) -> RelationshipManager* {
                         return ctx->m_relationship_manager.get();
                     })
        .addFunction("m_animation_player_manager",
                     +[](GameContext* ctx) -> AnimationPlayerManager* {
                         return ctx->m_animation_player_manager.get();
                     })
        .addFunction("m_ui_manager",
                     +[](GameContext* ctx) -> UIComponentManager* {
                         return ctx->m_ui_manager.get();
                     })
        .addFunction("m_gameplay_config_manager",
                     +[](GameContext* ctx) -> GameplayConfigManager* {
                         return ctx->m_gameplay_config_manager.get();
                     })
        .addFunction("m_tilemap_component_manager",
                     +[](GameContext* ctx) -> TilemapComponentManager* {
                         return ctx->m_tilemap_component_manager.get();
                     })
        .endClass()
        .endNamespace();
}

static void bindAssetsManager(lua_State* L) {
    getGlobalNamespace(L)
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

static void bindTimer(lua_State* L) {
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<Time>("Time")
        .addFunction("GetElapseTime", &Time::GetElapseTime)
        .addFunction("GetCurrentTime", &Time::GetCurrentTime)
        .endClass()
        .endNamespace();
}

static void bindCamera(lua_State* L) {
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
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

static void bindPrefabManager(lua_State* L) {
    getGlobalNamespace(L)
        .beginNamespace("TL")
        .beginClass<PrefabManager>("PrefabManager")
        .addFunction("Find", +[](PrefabManager* m, const std::string& path) {
            return m->Find(Path(path));
        })
        .endClass()
        .endNamespace();
}

static void bindFontManager(lua_State* L) {
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
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
    getGlobalNamespace(L)
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

static void bindAllTypes(lua_State* L) {
    bindScriptBinaryDataManager(L);
    bindEntity(L);
    bindPath(L);
    bindMath(L);
    bindLevel(L);
    bindImage(L);
    bindContext(L);
    bindAssetsManager(L);
    bindTimer(L);
    bindCamera(L);
    bindSprite(L);
    bindPrefabManager(L);
    bindLevelManager(L);
    bindFontManager(L);
    bindAnimationManager(L);
    bindTilemapManager(L);
}

void BindTLModule(lua_State* L) {
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "lua_State* is null!");

    getGlobalNamespace(L)
        .beginNamespace("TL")
        .addFunction("Log", LuauLog)
        .addFunction("Log", LuauLogInt)
        .addFunction("Log", LuauLogFloat)
        .addFunction("GetContext", +[]() -> GameContext* {
            return &GameContext::GetInst();
        })
        .endNamespace();

    bindAllTypes(L);
}
