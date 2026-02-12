#include "engine/script/script_binding.hpp"
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "imgui.h"
#include <vector>
#include "engine/animation_player.hpp"
#include "engine/asset_manager.hpp"
#include "engine/camera.hpp"
#include "engine/cct.hpp"
#include "engine/collision_group.hpp"
#include "engine/context.hpp"
#include "engine/debug_drawer.hpp"
#include "engine/entity.hpp"
#include "engine/flag.hpp"
#include "engine/gameplay_config.hpp"
#include "engine/image.hpp"
#include "engine/input/input.hpp"
#include "engine/level.hpp"
#include "engine/math.hpp"
#include "engine/path.hpp"
#include "engine/physics.hpp"
#include "engine/relationship.hpp"
#include "engine/renderer.hpp"
#include "engine/script/script.hpp"
#include "engine/script/script_array_binding.hpp"
#include "engine/script/script_flags_binding.hpp"
#include "engine/script/script_handle_binding.hpp"
#include "engine/script/script_macros.hpp"
#include "engine/script/script_option_binding.hpp"
#include "engine/script/script_template_binding.hpp"
#include "engine/sprite.hpp"
#include "engine/text.hpp"
#include "engine/tilemap.hpp"
#include "engine/timer.hpp"
#include "engine/transform.hpp"
#include "engine/trigger.hpp"
#include "engine/ui.hpp"
#include "engine/window.hpp"
#include "schema/binding/binding.hpp"
#include "schema/prefab.hpp"
#include "scriptany/scriptany.h"

template <typename T>
void AngelScriptLog(T msg) {
    LOGI("[Script]: {}", msg);
}

// Forward binding: register all types
void registerAllTypes(asIScriptEngine* engine) {
    // Optional<T> (CppOptional)
    registerOptionalType(engine);
    // Flags<T> (CppFlags) template for schema-generated types
    registerFlagsType(engine);
    // Handle<T> (CppHandle) template for schema-generated types and engine
    registerHandleType(engine);
    // reigster schema generate types
    RegisterSchemaType(engine);
    // Math types
    registerTVec2Type<float>(engine, "Vec2");
    AS_CALL(engine->RegisterObjectType("Degrees", sizeof(Degrees),
                                       asOBJ_VALUE | asOBJ_POD |
                                           asOBJ_APP_CLASS_ALLFLOATS |
                                           asGetTypeTraits<Degrees>()));
    AS_CALL(engine->RegisterObjectType("Radians", sizeof(Radians),
                                       asOBJ_VALUE | asOBJ_POD |
                                           asOBJ_APP_CLASS_ALLFLOATS |
                                           asGetTypeTraits<Radians>()));
    AS_CALL(engine->RegisterObjectType("Color", sizeof(Color),
                                       asOBJ_VALUE | asOBJ_POD |
                                           asOBJ_APP_CLASS_ALLFLOATS |
                                           asGetTypeTraits<Color>()));
    AS_CALL(
        engine->RegisterObjectType("Transform", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("TransformManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType(
        "Region", sizeof(Region),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Region>()));
    registerTVec2Type<uint32_t>(engine, "Vec2UI");

    // Path
    AS_CALL(engine->RegisterObjectType(
        "Path", sizeof(Path),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Path>()));

    // Entity and Level
    AS_CALL(engine->RegisterObjectType(
        "Entity", sizeof(Entity),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE));
    AS_CALL(engine->RegisterObjectType(
        "NullEntity", sizeof(NullEntity),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<NullEntity>()));
    AS_CALL(engine->RegisterObjectType("Level", 0, asOBJ_REF | asOBJ_NOCOUNT));

    // Image
    AS_CALL(engine->RegisterObjectType("Image", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("ImageManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Font
    AS_CALL(engine->RegisterObjectType("Font", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("FontManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Animation
    AS_CALL(
        engine->RegisterObjectType("Animation", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("AnimationManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("AnimationPlayer", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("AnimationPlayerManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // GameplayConfig
    AS_CALL(engine->RegisterObjectType("GameplayConfigManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Level
    AS_CALL(engine->RegisterObjectType("LevelManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Prefab
    AS_CALL(engine->RegisterObjectType("PrefabManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Script
    AS_CALL(engine->RegisterObjectType("ScriptBinaryData", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("ScriptBinaryDataManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("ScriptComponentManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Sprite
    AS_CALL(engine->RegisterObjectType("Sprite", sizeof(Sprite),
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("SpriteManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Tilemap
    AS_CALL(engine->RegisterObjectType(
        "Tile", sizeof(Tile),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Tile>()));
    AS_CALL(
        engine->RegisterObjectType("Tilemap", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("TilemapManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("TilemapComponent",
                                       sizeof(TilemapComponent),
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("TilemapComponentManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // UI
    AS_CALL(engine->RegisterObjectType("UIText", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("UIWidget", sizeof(UIWidget),
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("UIComponentManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Physics
    AS_CALL(engine->RegisterObjectType(
        "HitResult", sizeof(HitResult),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<HitResult>()));
    AS_CALL(engine->RegisterObjectType(
        "SweepResult", sizeof(SweepResult),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<SweepResult>()));
    AS_CALL(engine->RegisterObjectType(
        "OverlapResult", sizeof(OverlapResult),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<OverlapResult>()));
    AS_CALL(engine->RegisterObjectType(
        "PhysicsShape", sizeof(PhysicsShape),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<PhysicsShape>()));
    AS_CALL(engine->RegisterObjectType("PhysicsActor", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("PhysicsScene", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("CharacterController", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(
        engine->RegisterObjectType("CCTManager", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType(
        "CollisionGroup", sizeof(CollisionGroup),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<CollisionGroup>()));

    // Relationship
    AS_CALL(engine->RegisterObjectType("Relationship", sizeof(Relationship),
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("RelationshipManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Window and Renderer
    AS_CALL(engine->RegisterObjectType("Window", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType(
        "Image9Grid", sizeof(Image9Grid),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Image9Grid>()));
    AS_CALL(
        engine->RegisterObjectType("Renderer", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("DebugDrawer", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Timer
    AS_CALL(engine->RegisterObjectType("Time", 0, asOBJ_REF | asOBJ_NOCOUNT));
    // TimeType is an alias for double
    AS_CALL(engine->RegisterTypedef("TimeType", "double"));
    AS_CALL(engine->RegisterObjectType("Timer", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("TimerManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Input
    AS_CALL(engine->RegisterObjectType("Action", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType("Axis", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType(
        "Axises", sizeof(Axises), asOBJ_VALUE | asGetTypeTraits<Axises>()));
    AS_CALL(engine->RegisterObjectType("InputManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Trigger
    AS_CALL(engine->RegisterObjectType(
        "TriggerEnterEvent", sizeof(TriggerEnterEvent),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerEnterEvent>()));
    AS_CALL(engine->RegisterObjectType(
        "TriggerLeaveEvent", sizeof(TriggerLeaveEvent),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerLeaveEvent>()));
    AS_CALL(engine->RegisterObjectType(
        "TriggerTouchEvent", sizeof(TriggerTouchEvent),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerTouchEvent>()));
    AS_CALL(engine->RegisterObjectType(
        "Trigger", sizeof(Trigger), asOBJ_VALUE | asGetTypeTraits<Trigger>()));
    AS_CALL(engine->RegisterObjectBehaviour("Trigger", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Trigger, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Trigger", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(+[](Trigger* obj) { obj->~Trigger(); }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectType("TriggerComponentManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Context
    AS_CALL(engine->RegisterObjectType("GameContext", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));
    AS_CALL(engine->RegisterObjectType(
        "Camera", sizeof(Camera),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Camera>()));

    // AssetsManager
    AS_CALL(engine->RegisterObjectType("AssetsManager", 0,
                                       asOBJ_REF | asOBJ_NOCOUNT));

    // Enum types
    AS_CALL(engine->RegisterEnum("HitType"));
    AS_CALL(engine->RegisterEnumValue("HitType", "None",
                                      static_cast<int>(HitType::None)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Left",
                                      static_cast<int>(HitType::Left)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Right",
                                      static_cast<int>(HitType::Right)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Top",
                                      static_cast<int>(HitType::Top)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Bottom",
                                      static_cast<int>(HitType::Bottom)));

    AS_CALL(engine->RegisterEnum("PhysicsShapeType"));
    AS_CALL(
        engine->RegisterEnumValue("PhysicsShapeType", "Unknown",
                                  static_cast<int>(PhysicsShapeType::Unknown)));
    AS_CALL(engine->RegisterEnumValue(
        "PhysicsShapeType", "Rect", static_cast<int>(PhysicsShapeType::Rect)));
    AS_CALL(
        engine->RegisterEnumValue("PhysicsShapeType", "Circle",
                                  static_cast<int>(PhysicsShapeType::Circle)));

    AS_CALL(engine->RegisterObjectType(
        "TimerID", sizeof(TimerID),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE));

    // script inner usage types
    AS_CALL(engine->RegisterInterface("IBehavior"));
    AS_CALL(engine->RegisterInterfaceMethod("IBehavior",
                                            "Entity GetEntity() const"));
    AS_CALL(engine->RegisterInterfaceMethod("IBehavior", "void OnInit()"));
    AS_CALL(engine->RegisterInterfaceMethod("IBehavior",
                                            "void OnUpdate(TimeType)"));
    AS_CALL(engine->RegisterInterfaceMethod("IBehavior",
                                            "void OnRender()"));
    AS_CALL(engine->RegisterInterfaceMethod("IBehavior", "void OnQuit()"));
}

void bindDegrees(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectBehaviour("Degrees", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Degrees, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Degrees", asBEHAVE_CONSTRUCT, "void f(float)",
        WRAP_CON(Degrees, (float)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Degrees", asBEHAVE_CONSTRUCT, "void f(Radians)",
        WRAP_CON(Degrees, (Radians)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("Degrees", "float Value() const",
                                         asMETHOD(Degrees, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "bool opEquals(Degrees) const",
        asMETHODPR(Degrees, operator==, (Degrees) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Degrees", "int opCmp(Degrees) const",
                                         asFUNCTION(+[](Degrees a, Degrees b) {
                                             return a.Value() < b.Value()   ? -1
                                                    : a.Value() > b.Value() ? 1
                                                                            : 0;
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opAddAssign(Degrees)",
        asMETHODPR(Degrees, operator+=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opSubAssign(Degrees)",
        asMETHODPR(Degrees, operator-=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opMulAssign(Degrees)",
        asMETHODPR(Degrees, operator*=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opDivAssign(Degrees)",
        asMETHODPR(Degrees, operator/=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Degrees", "Degrees opAdd(Degrees)",
                                         asMETHOD(Degrees, operator+),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Degrees", "Degrees opSub(Degrees)",
                                         asMETHOD(Degrees, operator-),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opMul(Degrees)",
        asMETHODPR_CONST(Degrees, operator*, (Degrees), Degrees),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opDiv(Degrees)",
        asMETHODPR_CONST(Degrees, operator/, (Degrees), Degrees),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opMul_r(float)",
        asFUNCTIONPR(operator*, (float, Degrees), Degrees),
        asCALL_CDECL_OBJLAST));
}

void bindRadians(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectBehaviour("Radians", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Radians, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Radians", asBEHAVE_CONSTRUCT, "void f(float)",
        WRAP_CON(Radians, (float)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Radians", asBEHAVE_CONSTRUCT, "void f(Degrees)",
        WRAP_CON(Radians, (Degrees)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("Radians", "float Value() const",
                                         asMETHOD(Radians, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "bool opEquals(Radians) const",
        asMETHODPR(Radians, operator==, (Radians) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Radians", "int opCmp(Radians) const",
                                         asFUNCTION(+[](Radians a, Radians b) {
                                             return a.Value() < b.Value()   ? -1
                                                    : a.Value() > b.Value() ? 1
                                                                            : 0;
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opAddAssign(Radians)",
        asMETHODPR(Radians, operator+=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opSubAssign(Radians)",
        asMETHODPR(Radians, operator-=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opMulAssign(Radians)",
        asMETHODPR(Radians, operator*=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opDivAssign(Radians)",
        asMETHODPR(Radians, operator/=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Radians", "Radians opAdd(Radians)",
                                         asMETHOD(Radians, operator+),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Radians", "Radians opSub(Radians)",
                                         asMETHOD(Radians, operator-),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opMul(Radians)",
        asMETHODPR_CONST(Radians, operator*, (Radians), Radians),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opDiv(Radians)",
        asMETHODPR_CONST(Radians, operator/, (Radians), Radians),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opMul_r(float)",
        asFUNCTIONPR(operator*, (float, Radians), Radians),
        asCALL_CDECL_OBJLAST));
}

void bindColor(asIScriptEngine* engine) {
    AS_CALL(
        engine->RegisterObjectProperty("Color", "float r", asOFFSET(Color, r)));
    AS_CALL(
        engine->RegisterObjectProperty("Color", "float g", asOFFSET(Color, g)));
    AS_CALL(
        engine->RegisterObjectProperty("Color", "float b", asOFFSET(Color, b)));
    AS_CALL(
        engine->RegisterObjectProperty("Color", "float a", asOFFSET(Color, a)));
    AS_CALL(engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Color, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Color", asBEHAVE_CONSTRUCT, "void f(float, float, float)",
        WRAP_CON(Color, (float, float, float)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Color", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)",
        WRAP_CON(Color, (float, float, float, float)), asCALL_GENERIC));

    AS_CALL(engine->SetDefaultNamespace("TL::Color"));
    AS_CALL(engine->RegisterGlobalProperty("const Color Red",
                                           const_cast<Color*>(&Color::Red)));
    AS_CALL(engine->RegisterGlobalProperty("const Color Green",
                                           const_cast<Color*>(&Color::Green)));
    AS_CALL(engine->RegisterGlobalProperty("const Color Blue",
                                           const_cast<Color*>(&Color::Blue)));
    AS_CALL(engine->RegisterGlobalProperty("const Color Black",
                                           const_cast<Color*>(&Color::Black)));
    AS_CALL(engine->RegisterGlobalProperty("const Color White",
                                           const_cast<Color*>(&Color::White)));
    AS_CALL(engine->RegisterGlobalProperty("const Color Yellow",
                                           const_cast<Color*>(&Color::Yellow)));
    AS_CALL(engine->RegisterGlobalProperty("const Color Purple",
                                           const_cast<Color*>(&Color::Purple)));
    AS_CALL(engine->SetDefaultNamespace("TL"));
}

void bindTransform(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectProperty("Transform", "Vec2 m_position",
                                           asOFFSET(Transform, m_position)));
    AS_CALL(engine->RegisterObjectProperty("Transform", "Degrees m_rotation",
                                           asOFFSET(Transform, m_rotation)));
    AS_CALL(engine->RegisterObjectProperty("Transform", "Vec2 m_size",
                                           asOFFSET(Transform, m_size)));
    AS_CALL(engine->RegisterObjectProperty("Transform", "Degrees m_scale",
                                           asOFFSET(Transform, m_scale)));

    AS_CALL(engine->RegisterObjectMethod(
        "Transform", "bool opEquals(const Transform&in) const",
        asMETHODPR(Transform, operator==, (const Transform&) const, bool),
        asCALL_THISCALL));

    AS_CALL(engine->RegisterObjectMethod(
        "TransformManager", "Transform@ Get(Entity)",
        asMETHODPR(TransformManager, Get, (Entity), Transform*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "TransformManager", "const Transform@ Get(Entity) const",
            asMETHODPR_CONST(TransformManager, Get, (Entity), Transform* const),
            asCALL_THISCALL));
}

void bindOtherMath(asIScriptEngine* engine) {
    engine->RegisterGlobalFunction("Vec2 Rotate(const Vec2& in, Degrees)",
                                   asFUNCTION(Rotate), asCALL_CDECL);
    engine->RegisterGlobalFunction(
        "Radians GetAngle(const Vec2& in norm_a, const Vec2& in norm_b)",
        asFUNCTION(GetAngle), asCALL_CDECL);

    // Region
    AS_CALL(engine->RegisterObjectProperty("Region", "Vec2 m_topleft",
                                           asOFFSET(Region, m_topleft)));
    AS_CALL(engine->RegisterObjectProperty("Region", "Vec2 m_size",
                                           asOFFSET(Region, m_size)));
}

void bindPath(asIScriptEngine* engine) {
    // Constructors
    AS_CALL(engine->RegisterObjectBehaviour("Path", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Path, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Path", asBEHAVE_CONSTRUCT, "void f(const string& in)",
        WRAP_CON(Path, (const std::string&)), asCALL_GENERIC));

    // String conversion
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "string string() const",
        asMETHODPR(Path, string, () const, std::string), asCALL_THISCALL));

    // Path operations
    AS_CALL(engine->RegisterObjectMethod("Path", "Path parent_path() const",
                                         asMETHOD(Path, parent_path),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Path", "Path filename() const",
                                         asMETHOD(Path, filename),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Path", "Path extension() const",
                                         asMETHOD(Path, extension),
                                         asCALL_THISCALL));

    // Path queries
    AS_CALL(engine->RegisterObjectMethod("Path", "bool has_extension() const",
                                         asMETHOD(Path, has_extension),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Path", "bool is_absolute() const",
                                         asMETHOD(Path, is_absolute),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Path", "bool is_relative() const",
                                         asMETHOD(Path, is_relative),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "bool empty() const", asMETHOD(Path, empty), asCALL_THISCALL));

    // Operators
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "bool opEquals(const Path& in) const",
        asFUNCTION(
            +[](const Path& a, const Path& b) -> bool { return a == b; }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "bool opNotEquals(const Path& in) const",
        asFUNCTION(
            +[](const Path& a, const Path& b) -> bool { return a != b; }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "Path opDiv(const Path& in) const",
        asFUNCTION(+[](const Path& a, const Path& b) -> Path { return a / b; }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "Path opDiv(const string& in) const",
        asFUNCTION(
            +[](const Path& p, const std::string& s) -> Path { return p / s; }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "Path& opDivAssign(const Path& in)",
        asMETHODPR(Path, operator/=, (const Path&), Path&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Path", "Path& opDivAssign(const string& in)",
        asFUNCTION(+[](Path& p, const std::string& s) -> Path& {
            p /= s;
            return p;
        }),
        asCALL_CDECL_OBJFIRST));
}

static void EntityConstructDefault(asIScriptGeneric* gen) {
    *static_cast<Entity*>(gen->GetObject()) = null_entity;
}

void bindEntity(asIScriptEngine* engine) {
    // Entity default construct: assign null_entity (must use generic to get
    // object pointer)
    AS_CALL(engine->RegisterObjectBehaviour(
        "Entity", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(EntityConstructDefault), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod(
        "Entity", "bool IsNull() const",
        asFUNCTION(+[](const Entity* entity) -> bool {
            return *entity == null_entity;
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindLevel(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "Level", "Entity Instantiate(Handle<Prefab>)",
        asFUNCTION(+[](Level* level, const CppHandle& h) -> Entity {
            Handle<Prefab> prefab;
            HandleFromCppHandle<Prefab>(&h, prefab);
            return level->Instantiate(prefab);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("Level", "void RemoveEntity(Entity)",
                                         asMETHOD(Level, RemoveEntity),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Level", "Entity GetRootEntity() const", asMETHOD(Level, GetRootEntity),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Level", "Entity GetUIRootEntity() const",
        asMETHOD(Level, GetUIRootEntity), asCALL_THISCALL));
}

void bindImage(asIScriptEngine* engine) {
    // Image methods
    AS_CALL(engine->RegisterObjectMethod("Image", "Vec2 GetSize() const",
                                         asMETHOD(Image, GetSize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Image", "void ChangeColorMask(const Color& in)",
        asMETHOD(Image, ChangeColorMask), asCALL_THISCALL));

    // ImageManager
    AS_CALL(engine->RegisterObjectMethod(
        "ImageManager", "Handle<Image> Load(const string& in, bool)",
        asFUNCTION(+[](ImageManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Image>(asGetActiveContext()->GetEngine(),
                                              mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "ImageManager", "Handle<Image> Find(const string& in)",
        asFUNCTION(+[](ImageManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Image>(asGetActiveContext()->GetEngine(),
                                              mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));
}

// Helper function: get ImageManager instance
ImageManager& GetImageManager() {
    return CURRENT_CONTEXT.m_assets_manager->GetManager<Image>();
}

void bindFont(asIScriptEngine* engine) {
    // Font methods
    AS_CALL(engine->RegisterObjectMethod("Font", "int GetHeight() const",
                                         asMETHOD(Font, GetHeight),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Font", "void SetFontSize(int)",
                                         asMETHOD(Font, SetFontSize),
                                         asCALL_THISCALL));

    // FontManager
    AS_CALL(engine->RegisterObjectMethod(
        "FontManager", "Handle<Font> Load(const string& in, bool)",
        asFUNCTION(+[](FontManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Font>(asGetActiveContext()->GetEngine(),
                                             mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "FontManager", "Handle<Font> Find(const string& in)",
        asFUNCTION(+[](FontManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Font>(asGetActiveContext()->GetEngine(),
                                             mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindAnimationManager(asIScriptEngine* engine) {
    // AnimationManager
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationManager", "Handle<Animation> Load(const string& in, bool)",
        asFUNCTION(+[](AnimationManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Animation>(
                asGetActiveContext()->GetEngine(),
                mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationManager", "Handle<Animation> Create()",
        asFUNCTION(+[](AnimationManager* mgr) -> CppHandle {
            return CppHandleFromHandle<Animation>(
                asGetActiveContext()->GetEngine(), mgr->Create());
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationManager", "Handle<Animation> Find(const string& in)",
        asFUNCTION(+[](AnimationManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Animation>(
                asGetActiveContext()->GetEngine(), mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindAnimationPlayer(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void Play()",
                                         asMETHOD(AnimationPlayer, Play),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void Pause()",
                                         asMETHOD(AnimationPlayer, Pause),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void Stop()",
                                         asMETHOD(AnimationPlayer, Stop),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void Rewind()",
                                         asMETHOD(AnimationPlayer, Rewind),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void SetLoop(int)",
                                         asMETHOD(AnimationPlayer, SetLoop),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "bool IsPlaying() const",
        asMETHOD(AnimationPlayer, IsPlaying), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "int GetLoopCount() const",
        asMETHOD(AnimationPlayer, GetLoopCount), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "TimeType GetCurTime() const",
        asMETHOD(AnimationPlayer, GetCurTime), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "TimeType GetMaxTime() const",
        asMETHOD(AnimationPlayer, GetMaxTime), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void ChangeAnimation(Handle<Animation>)",
        asFUNCTION(+[](AnimationPlayer* player, const CppHandle& h) {
            AnimationHandle anim;
            HandleFromCppHandle<Animation>(&h, anim);
            player->ChangeAnimation(anim);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void ChangeAnimation(const string& in)",
        asFUNCTION(+[](AnimationPlayer* player, const std::string& filename) {
            player->ChangeAnimation(Path(filename));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void ClearAnimation()",
        asMETHOD(AnimationPlayer, ClearAnimation), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "bool HasAnimation() const",
        asMETHOD(AnimationPlayer, HasAnimation), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void Update(TimeType)",
        asMETHOD(AnimationPlayer, Update), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("AnimationPlayer", "void Sync(Entity)",
                                         asMETHOD(AnimationPlayer, Sync),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void SetRate(float)",
        asMETHOD(AnimationPlayer, SetRate), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "float GetRate() const",
        asMETHOD(AnimationPlayer, GetRate), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "Handle<Animation> GetAnimation() const",
        asFUNCTION(+[](AnimationPlayer* player) -> CppHandle {
            return CppHandleFromHandle<Animation>(
                asGetActiveContext()->GetEngine(), player->GetAnimation());
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "void EnableAutoPlay(bool)",
        asMETHOD(AnimationPlayer, EnableAutoPlay), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayer", "bool IsAutoPlayEnabled() const",
        asMETHOD(AnimationPlayer, IsAutoPlayEnabled), asCALL_THISCALL));

    // AnimationPlayerManager
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayerManager", "AnimationPlayer@ Get(Entity)",
        asMETHODPR(AnimationPlayerManager, Get, (Entity), AnimationPlayer*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "AnimationPlayerManager", "const AnimationPlayer@ Get(Entity) const",
            asMETHODPR_CONST(AnimationPlayerManager, Get, (Entity), AnimationPlayer* const),
            asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "AnimationPlayerManager", "void Update(TimeType)",
        asMETHOD(AnimationPlayerManager, Update), asCALL_THISCALL));
}

void bindGameplayConfigManager(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "GameplayConfigManager", "GameplayConfig@ Get(Entity)",
        asMETHODPR(GameplayConfigManager, Get, (Entity), GameplayConfig*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "GameplayConfigManager", "const GameplayConfig@ Get(Entity) const",
            asMETHODPR_CONST(GameplayConfigManager, Get, (Entity), GameplayConfig* const),
            asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "GameplayConfigManager", "void Update(TimeType)",
        asMETHOD(GameplayConfigManager, Update), asCALL_THISCALL));
}

void bindLevelManager(asIScriptEngine* engine) {
    // LevelManager
    AS_CALL(engine->RegisterObjectMethod(
        "LevelManager", "Handle<Level> Load(const string& in, bool)",
        asFUNCTION(+[](LevelManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Level>(asGetActiveContext()->GetEngine(),
                                              mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "LevelManager", "Handle<Level> Find(const string& in)",
        asFUNCTION(+[](LevelManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Level>(asGetActiveContext()->GetEngine(),
                                              mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindPrefabManager(asIScriptEngine* engine) {
    // PrefabManager
    AS_CALL(engine->RegisterObjectMethod(
        "PrefabManager", "Handle<Prefab> Load(const string& in, bool)",
        asFUNCTION(+[](PrefabManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Prefab>(
                asGetActiveContext()->GetEngine(),
                mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PrefabManager", "Handle<Prefab> Create()",
        asFUNCTION(+[](PrefabManager* mgr) -> CppHandle {
            return CppHandleFromHandle<Prefab>(
                asGetActiveContext()->GetEngine(), mgr->Create());
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PrefabManager", "Handle<Prefab> Find(const string& in)",
        asFUNCTION(+[](PrefabManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Prefab>(
                asGetActiveContext()->GetEngine(), mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindScriptBinaryDataManager(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "ScriptBinaryDataManager",
        "Handle<ScriptBinaryData> Load(const string& in, bool)",
        asFUNCTION(+[](ScriptBinaryDataManager* mgr,
                       const std::string& filename, bool force) -> CppHandle {
            return CppHandleFromHandle<ScriptBinaryData>(
                asGetActiveContext()->GetEngine(),
                mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "ScriptBinaryDataManager",
        "Handle<ScriptBinaryData> Find(const string& in)",
        asFUNCTION(+[](ScriptBinaryDataManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<ScriptBinaryData>(
                asGetActiveContext()->GetEngine(), mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));

    AS_CALL(engine->RegisterObjectMethod(
        "ScriptComponentManager", "IBehavior@ Get(Entity)",
        asFUNCTION(+[](ScriptComponentManager* mgr,
                       Entity entity) -> asIScriptObject* {
            Script* s = mgr->Get(entity);
            asIScriptObject* obj = s ? s->GetScriptObject() : nullptr;
            if (obj) {
                obj->AddRef();
            }
            return obj;
        }),
        asCALL_CDECL_OBJFIRST));


    AS_CALL(engine->RegisterObjectMethod(
            "ScriptComponentManager", "const IBehavior@ Get(Entity) const",
            asFUNCTION(+[](ScriptComponentManager* mgr,
                           Entity entity) -> const asIScriptObject* {
                Script* s = mgr->Get(entity);
                asIScriptObject* obj = s ? s->GetScriptObject() : nullptr;
                if (obj) {
                    obj->AddRef();
                }
                return obj;
            }),
            asCALL_CDECL_OBJFIRST));
}

void bindSprite(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectProperty(
        "Sprite", "Region m_region", asOFFSET(SpriteDefinition, m_region)));
    AS_CALL(engine->RegisterObjectProperty("Sprite", "Vec2 m_size",
                                           asOFFSET(SpriteDefinition, m_size)));
    AS_CALL(engine->RegisterObjectProperty(
        "Sprite", "Vec2 m_anchor", asOFFSET(SpriteDefinition, m_anchor)));
    AS_CALL(engine->RegisterObjectProperty(
        "Sprite", "float m_z_order", asOFFSET(SpriteDefinition, m_z_order)));
    AS_CALL(engine->RegisterObjectProperty(
        "Sprite", "Color m_color", asOFFSET(SpriteDefinition, m_color)));

    AS_CALL(engine->RegisterObjectMethod(
        "Sprite", "Flags<Flip> get_m_flip() const property",
        asFUNCTION(+[](const SpriteDefinition* p) {
            return CppFlagsFromFlags<Flip>(asGetActiveContext()->GetEngine(),
                                           p->m_flip);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Sprite", "void set_m_flip(const Flags<Flip>& in) property",
        asFUNCTION(+[](SpriteDefinition* p, const CppFlags& o) {
            FlagsFromCppFlags<Flip>(&o, p->m_flip);
        }),
        asCALL_CDECL_OBJFIRST));

    AS_CALL(engine->RegisterObjectMethod(
        "Sprite", "Handle<Image> get_m_image() const property",
        asFUNCTION(+[](const SpriteDefinition* p) {
            return CppHandleFromHandle<Image>(asGetActiveContext()->GetEngine(),
                                              p->m_image);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Sprite", "void set_m_image(Handle<Image>) property",
        asFUNCTION(+[](SpriteDefinition* p, const CppHandle& o) {
            HandleFromCppHandle<Image>(&o, p->m_image);
        }),
        asCALL_CDECL_OBJFIRST));

    AS_CALL(engine->RegisterObjectMethod(
        "SpriteManager", "Sprite@ Get(Entity)",
        asMETHODPR(SpriteManager, Get, (Entity), Sprite*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "SpriteManager", "const Sprite@ Get(Entity) const",
            asMETHODPR_CONST(SpriteManager, Get, (Entity), Sprite* const),
            asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("SpriteManager", "void Update()",
                                         asMETHOD(SpriteManager, Update),
                                         asCALL_THISCALL));
}

void bindTilemap(asIScriptEngine* engine) {
    // Tile structure
    AS_CALL(engine->RegisterObjectMethod(
        "Tile", "Handle<Image> get_image() const property",
        asFUNCTION(+[](Tile* t) {
            return CppHandleFromHandle<Image>(asGetActiveContext()->GetEngine(),
                                              t->m_image);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Tile", "void set_image(Handle<Image>) property",
        asFUNCTION(+[](Tile* t, const CppHandle& o) {
            HandleFromCppHandle<Image>(&o, t->m_image);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectProperty("Tile", "Region m_region",
                                           asOFFSET(Tile, m_region)));
    AS_CALL(engine->RegisterObjectProperty("Tile", "uint32 m_id",
                                           asOFFSET(Tile, m_id)));

    // Tilemap
    AS_CALL(engine->RegisterObjectMethod(
        "Tilemap", "const Vec2& GetTileSize() const",
        asMETHOD(Tilemap, GetTileSize), asCALL_THISCALL));

    // TilemapManager
    AS_CALL(engine->RegisterObjectMethod(
        "TilemapManager", "Handle<Tilemap> Load(const string& in, bool)",
        asFUNCTION(+[](TilemapManager* mgr, const std::string& filename,
                       bool force) -> CppHandle {
            return CppHandleFromHandle<Tilemap>(
                asGetActiveContext()->GetEngine(),
                mgr->Load(Path(filename), force));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "TilemapManager", "Handle<Tilemap> Find(const string& in)",
        asFUNCTION(+[](TilemapManager* mgr,
                       const std::string& filename) -> CppHandle {
            return CppHandleFromHandle<Tilemap>(
                asGetActiveContext()->GetEngine(), mgr->Find(Path(filename)));
        }),
        asCALL_CDECL_OBJFIRST));

    // TilemapComponent
    AS_CALL(engine->RegisterObjectMethod(
        "TilemapComponent", "Handle<Tilemap> get_handle() const property",
        asFUNCTION(+[](TilemapComponent* c) {
            return CppHandleFromHandle<Tilemap>(
                asGetActiveContext()->GetEngine(), c->GetHandle());
        }),
        asCALL_CDECL_OBJFIRST));

    // TilemapComponentManager
    AS_CALL(engine->RegisterObjectMethod(
        "TilemapComponentManager", "void Update()",
        asMETHOD(TilemapComponentManager, Update), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "TilemapComponentManager", "TilemapComponent@ Get(Entity)",
        asMETHODPR(TilemapComponentManager, Get, (Entity), TilemapComponent*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "TilemapComponentManager", "const TilemapComponent@ Get(Entity) const",
            asMETHODPR_CONST(TilemapComponentManager, Get, (Entity), TilemapComponent* const),
            asCALL_THISCALL));
}

void bindUI(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectProperty("UIText", "UITextAlign m_align",
                                           asOFFSET(UIText, m_align)));
    AS_CALL(engine->RegisterObjectProperty("UIText", "bool m_resize_by_text",
                                           asOFFSET(UIText, m_resize_by_text)));
    AS_CALL(engine->RegisterObjectProperty("UIText", "Color m_color",
                                           asOFFSET(UIText, m_color)));
    AS_CALL(engine->RegisterObjectMethod(
        "UIText", "void SetFont(Handle<Font>)",
        asFUNCTION(+[](UIText* text, const CppHandle& h) {
            Handle<Font> font;
            HandleFromCppHandle<Font>(&h, font);
            text->SetFont(font);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "UIText", "void ChangeText(const string& in)",
        asMETHOD(UIText, ChangeText), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIText", "void ChangeTextPt(uint32)",
                                         asMETHOD(UIText, ChangeTextPt),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "UIText", "Vec2 GetTextImageSize() const",
        asMETHOD(UIText, GetTextImageSize), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "UIText", "const string& GetText() const", asMETHOD(UIText, GetText),
        asCALL_THISCALL));

    // UIWidget
    AS_CALL(engine->RegisterObjectMethod(
        "UIWidget", "Flags<UIAnchor> get_anchor() const property",
        asFUNCTION(+[](UIWidget* w) {
            return CppFlagsFromFlags<UIAnchor>(
                asGetActiveContext()->GetEngine(), w->m_anchor);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "UIWidget", "void set_anchor(const Flags<UIAnchor>& in) property",
        asFUNCTION(+[](UIWidget* w, const CppFlags& o) {
            FlagsFromCppFlags<UIAnchor>(&o, w->m_anchor);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_use_clip",
                                           asOFFSET(UIWidget, m_use_clip)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_disabled",
                                           asOFFSET(UIWidget, m_disabled)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_selected",
                                           asOFFSET(UIWidget, m_selected)));
    AS_CALL(
        engine->RegisterObjectProperty("UIWidget", "bool m_can_be_selected",
                                       asOFFSET(UIWidget, m_can_be_selected)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "UIState m_state",
                                           asOFFSET(UIWidget, m_state)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "Vec2 m_margin",
                                           asOFFSET(UIWidget, m_margin)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "Vec2 m_padding",
                                           asOFFSET(UIWidget, m_padding)));

    // UIComponentManager
    AS_CALL(engine->RegisterObjectMethod(
        "UIComponentManager", "UIWidget@ Get(Entity)",
        asMETHODPR(UIComponentManager, Get, (Entity), UIWidget*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "UIComponentManager", "const UIWidget@ Get(Entity) const",
            asMETHODPR_CONST(UIComponentManager, Get, (Entity), UIWidget* const),
            asCALL_THISCALL));
}

void bindCamera(asIScriptEngine* engine) {
    // Camera
    AS_CALL(engine->RegisterObjectMethod(
        "Camera", "void ChangeScale(const Vec2& in)",
        asMETHOD(Camera, ChangeScale), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Camera", "void MoveTo(const Vec2& in)", asMETHOD(Camera, MoveTo),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Camera", "void Move(const Vec2& in)",
                                         asMETHOD(Camera, Move),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Camera", "const Vec2& GetScale() const", asMETHOD(Camera, GetScale),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Camera", "const Vec2& GetPosition() const",
        asMETHOD(Camera, GetPosition), asCALL_THISCALL));
}

void bindCollisionGroup(asIScriptEngine* engine) {
    // CollisionGroup
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "void Add(CollisionGroupType)",
        asMETHOD(CollisionGroup, Add), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "void Remove(CollisionGroupType)",
        asMETHOD(CollisionGroup, Remove), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "bool Has(CollisionGroupType) const",
        asMETHOD(CollisionGroup, Has), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("CollisionGroup", "void Clear()",
                                         asMETHOD(CollisionGroup, Clear),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "bool CanCollision(const CollisionGroup& in) const",
        asMETHOD(CollisionGroup, CanCollision), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "uint32 GetUnderlying() const",
        asMETHOD(CollisionGroup, GetUnderlying), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CollisionGroup", "void SetUnderlying(uint32)",
        asMETHOD(CollisionGroup, SetUnderlying), asCALL_THISCALL));
}

void bindPhysics(asIScriptEngine* engine) {
    // HitResult
    AS_CALL(engine->RegisterObjectProperty("HitResult", "float m_t",
                                           asOFFSET(HitResult, m_t)));
    AS_CALL(engine->RegisterObjectMethod(
        "HitResult", "Flags<HitType> get_flags() const property",
        asFUNCTION(+[](HitResult* h) {
            return CppFlagsFromFlags<HitType>(asGetActiveContext()->GetEngine(),
                                              h->m_flags);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "HitResult", "void set_flags(const Flags<HitType>& in) property",
        asFUNCTION(+[](HitResult* h, const CppFlags& o) {
            FlagsFromCppFlags<HitType>(&o, h->m_flags);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectProperty("HitResult", "Vec2 m_normal",
                                           asOFFSET(HitResult, m_normal)));
    AS_CALL(engine->RegisterObjectProperty(
        "HitResult", "bool m_is_initial_overlap",
        asOFFSET(HitResult, m_is_initial_overlap)));

    // SweepResult
    AS_CALL(engine->RegisterObjectProperty("SweepResult", "Entity m_entity",
                                           asOFFSET(SweepResult, m_entity)));

    // OverlapResult
    AS_CALL(
        engine->RegisterObjectProperty("OverlapResult", "Entity m_dst_entity",
                                       asOFFSET(OverlapResult, m_dst_entity)));

    // PhysicsShape
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "const Rect& AsRect() const",
        asFUNCTION(+[](const PhysicsShape* shape) -> const Rect& {
            static Rect empty_rect{};
            const Rect* rect = shape->AsRect();
            return rect ? *rect : empty_rect;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "const Circle& AsCircle() const",
        asFUNCTION(+[](const PhysicsShape* shape) -> const Circle& {
            static Circle empty_circle{};
            const Circle* circle = shape->AsCircle();
            return circle ? *circle : empty_circle;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "PhysicsShapeType GetType() const",
        asMETHOD(PhysicsShape, GetType), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "const Vec2& GetPosition() const",
        asMETHOD(PhysicsShape, GetPosition), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "void MoveTo(const Vec2& in)",
        asMETHOD(PhysicsShape, MoveTo), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsShape", "void Move(const Vec2& in)",
        asMETHOD(PhysicsShape, Move), asCALL_THISCALL));

    // PhysicsActor
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsActor", "const PhysicsShape& GetShape() const",
        asMETHOD(PhysicsActor, GetShape), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsActor", "const Vec2& GetPosition() const",
        asMETHOD(PhysicsActor, GetPosition), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsActor", "Entity GetEntity() const",
        asMETHOD(PhysicsActor, GetEntity), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsActor", "void MoveTo(const Vec2& in)",
        asMETHOD(PhysicsActor, MoveTo), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsActor", "void Move(const Vec2& in)",
        asMETHOD(PhysicsActor, Move), asCALL_THISCALL));

    // PhysicsScene
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "PhysicsActor@ CreateActor(Entity, const Rect& in)",
        asMETHODPR(PhysicsScene, CreateActor, (Entity, const Rect&),
                   PhysicsActor*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "PhysicsActor@ CreateActor(Entity, const Circle& in)",
        asMETHODPR(PhysicsScene, CreateActor, (Entity, const Circle&),
                   PhysicsActor*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "void RemoveActor(PhysicsActor@)",
        asMETHOD(PhysicsScene, RemoveActor), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene",
        "uint32 Sweep(const PhysicsActor& in, const Vec2& in, float, "
        "array<SweepResult>@)",
        asFUNCTION(+[](PhysicsScene* scene, const PhysicsActor& actor,
                       const Vec2& dir, float dist,
                       CScriptArray* arr) -> asUINT {
            if (!arr) return 0;
            uint32_t cnt = scene->Sweep(
                actor, dir, dist, static_cast<SweepResult*>(arr->GetBuffer()),
                static_cast<size_t>(arr->GetSize()));
            arr->Resize(cnt);
            return cnt;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene",
        "uint32 Overlap(const PhysicsActor& in, array<OverlapResult>@)",
        asFUNCTION(+[](PhysicsScene* scene, const PhysicsActor& actor,
                       CScriptArray* arr) -> asUINT {
            if (!arr) return 0;
            uint32_t cnt = scene->Overlap(
                actor, static_cast<OverlapResult*>(arr->GetBuffer()),
                static_cast<size_t>(arr->GetSize()));
            arr->Resize(cnt);
            return cnt;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene",
        "bool Overlap(const PhysicsActor& in, const PhysicsActor& in) const",
        asMETHODPR(PhysicsScene, Overlap,
                   (const PhysicsActor&, const PhysicsActor&) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "bool IsEnableDebugDraw() const",
        asMETHOD(PhysicsScene, IsEnableDebugDraw), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "void ToggleDebugDraw()",
        asMETHOD(PhysicsScene, ToggleDebugDraw), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "PhysicsScene", "void RenderDebug() const",
        asMETHOD(PhysicsScene, RenderDebug), asCALL_THISCALL));
}

void bindCharacterController(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "void MoveAndSlide(const Vec2& in)",
        asMETHOD(CharacterController, MoveAndSlide), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "Vec2 GetPosition() const",
        asMETHOD(CharacterController, GetPosition), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "void SetSkin(float)",
        asMETHOD(CharacterController, SetSkin), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "float GetSkin() const",
        asMETHOD(CharacterController, GetSkin), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "void SetMinDisp(float)",
        asMETHOD(CharacterController, SetMinDisp), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "float GetMinDisp() const",
        asMETHOD(CharacterController, GetMinDisp), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "void Teleport(const Vec2& in)",
        asMETHOD(CharacterController, Teleport), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "CharacterController", "const PhysicsActor@ GetActor() const",
        asMETHOD(CharacterController, GetActor), asCALL_THISCALL));

    // CCTManager
    AS_CALL(engine->RegisterObjectMethod(
        "CCTManager", "CharacterController@ Get(Entity)",
        asMETHODPR(CCTManager, Get, (Entity), CharacterController*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "CCTManager", "const CharacterController@ Get(Entity) const",
            asMETHODPR_CONST(CCTManager, Get, (Entity), CharacterController* const),
            asCALL_THISCALL));
}

void bindRelationship(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "Relationship", "array<Entity>@ get_m_children() const property",
        asFUNCTION(+[](const Relationship* p) {
            return VectorToScriptArray<Entity>(
                asGetActiveContext()->GetEngine(), p->m_children, "Entity");
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Relationship", "void set_m_children(array<Entity>@) property",
        asFUNCTION(+[](Relationship* p, CScriptArray* arr) {
            ScriptArrayToVector<Entity>(asGetActiveContext()->GetEngine(), arr,
                                        p->m_children);
        }),
        asCALL_CDECL_OBJFIRST));

    AS_CALL(engine->RegisterObjectMethod("RelationshipManager", "void Update()",
                                         asMETHOD(RelationshipManager, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "RelationshipManager", "Relationship@ Get(Entity)",
        asMETHODPR(RelationshipManager, Get, (Entity), Relationship*),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
            "RelationshipManager", "const Relationship@ Get(Entity) const",
            asMETHODPR_CONST(RelationshipManager, Get, (Entity), Relationship* const),
            asCALL_THISCALL));
}

void bindInputManager(asIScriptEngine* engine) {
    // Action
    AS_CALL(engine->RegisterObjectMethod("Action", "bool IsPressed(uint32) const",
                                         asMETHOD(Action, IsPressed),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Action", "bool IsPressing(uint32) const",
                                         asMETHOD(Action, IsPressing),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Action", "bool IsReleased(uint32) const",
                                         asMETHOD(Action, IsReleased),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Action", "bool IsReleasing(uint32) const", asMETHOD(Action, IsReleasing),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Action", "bool IsPress(uint32) const",
                                         asMETHOD(Action, IsPress),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Action", "bool IsRelease(uint32) const",
                                         asMETHOD(Action, IsRelease),
                                         asCALL_THISCALL));
    // Axis
    AS_CALL(engine->RegisterObjectMethod("Axis", "float Value(uint32) const",
                                         asMETHOD(Axis, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectBehaviour("Axises", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Axises, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Axises", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(+[](Axises* obj) { obj->~Axises(); }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("Axises", "Vec2 Value(uint32) const",
                                         asMETHOD(Axises, Value),
                                         asCALL_THISCALL));
    // InputManager
    AS_CALL(engine->RegisterObjectMethod(
        "InputManager", "Action@ GetAction(const string& in)",
        asFUNCTION(+[](InputManager* mgr, const std::string& name) -> Action* {
            return const_cast<Action*>(&mgr->GetAction(name));
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "InputManager", "const Axis@ GetAxis(const string& in) const",
        asFUNCTION(+[](const InputManager* mgr, const std::string& name) -> const Axis* {
            return &mgr->GetAxis(name);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
            "InputManager", "Axises MakeAxises(const string& in x_axis, const string& in y_axis) const",
            asFUNCTION(+[](const InputManager* mgr, const std::string& x_axis, const std::string& y_axis) -> Axises {
                return mgr->MakeAxises(x_axis, y_axis);
            }),
            asCALL_CDECL_OBJFIRST));
}

void bindWindow(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("Window", "Vec2 GetWindowSize() const",
                                         asMETHOD(Window, GetWindowSize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Window", "void SetTitle(const string& in)", asMETHOD(Window, SetTitle),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Window", "void Resize(const Vec2UI& in)", asMETHOD(Window, Resize),
        asCALL_THISCALL));
}

void bindRenderer(asIScriptEngine* engine) {
    // Image9Grid
    AS_CALL(engine->RegisterObjectProperty("Image9Grid", "float left",
                                           asOFFSET(Image9Grid, left)));
    AS_CALL(engine->RegisterObjectProperty("Image9Grid", "float right",
                                           asOFFSET(Image9Grid, right)));
    AS_CALL(engine->RegisterObjectProperty("Image9Grid", "float top",
                                           asOFFSET(Image9Grid, top)));
    AS_CALL(engine->RegisterObjectProperty("Image9Grid", "float bottom",
                                           asOFFSET(Image9Grid, bottom)));
    AS_CALL(engine->RegisterObjectProperty("Image9Grid", "float scale",
                                           asOFFSET(Image9Grid, scale)));
    AS_CALL(engine->RegisterObjectMethod("Image9Grid", "bool IsValid() const",
                                         asMETHOD(Image9Grid, IsValid),
                                         asCALL_THISCALL));

    // Renderer
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer", "void SetClearColor(const Color& in)",
        asMETHOD(Renderer, SetClearColor), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void DrawLine(const Vec2& in, const Vec2& in, const Color& in, float, "
        "bool)",
        asMETHODPR(Renderer, DrawLine,
                   (const Vec2&, const Vec2&, const Color&, float, bool), void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void DrawRect(const Rect& in, const Color& in, float, bool)",
        asMETHODPR(Renderer, DrawRect, (const Rect&, const Color&, float, bool),
                   void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void DrawCircle(const Circle& in, const Color& in, uint32, float, "
        "bool)",
        asMETHODPR(Renderer, DrawCircle,
                   (const Circle&, const Color&, uint32_t, float, bool), void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void FillRect(const Rect& in, const Color& in, float, bool)",
        asMETHODPR(Renderer, FillRect, (const Rect&, const Color&, float, bool),
                   void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void DrawImage(const Image& in, const Region& in, const Region& in, "
        "const Color& in, Degrees, const Vec2& in, const Flags<Flip>& in, "
        "float, bool)",
        asFUNCTION(+[](Renderer* r, const Image& img, const Region& src,
                       const Region& dst, const Color& color, Degrees rot,
                       const Vec2& center, const CppFlags& flip, float z_order,
                       bool use_camera) {
            Flags<Flip> cpp_flip;
            FlagsFromCppFlags<Flip>(&flip, cpp_flip);
            r->DrawImage(img, src, dst, color, rot, center, cpp_flip, z_order,
                         use_camera);
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Renderer",
        "void DrawImage9Grid(const Image& in, const Region& in, const Region& "
        "in, const Color& in, const Image9Grid& in, float, float, bool)",
        asMETHODPR(Renderer, DrawImage9Grid,
                   (const Image&, const Region&, const Region&, const Color&,
                    const Image9Grid&, float, float, bool),
                   void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void Clear()",
                                         asMETHOD(Renderer, Clear),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void Present()",
                                         asMETHOD(Renderer, Present),
                                         asCALL_THISCALL));
}

void bindDebugDrawer(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "DebugDrawer",
        "void DrawRect(const Rect& in, const Color& in, TimeType, bool)",
        asMETHODPR(DebugDrawer, DrawRect,
                   (const Rect&, const Color&, TimeType, bool), void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "DebugDrawer",
        "void DrawCircle(const Circle& in, const Color& in, TimeType, bool)",
        asMETHODPR(DebugDrawer, DrawCircle,
                   (const Circle&, const Color&, TimeType, bool), void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "DebugDrawer",
        "void FillRect(const Rect& in, const Color& in, TimeType, bool)",
        asMETHODPR(DebugDrawer, FillRect,
                   (const Rect&, const Color&, TimeType, bool), void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "DebugDrawer",
        "void AddLine(const Vec2& in, const Vec2& in, const Color& in, "
        "TimeType, bool)",
        asMETHODPR(DebugDrawer, AddLine,
                   (const Vec2&, const Vec2&, const Color&, TimeType, bool),
                   void),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("DebugDrawer", "void Clear()",
                                         asMETHOD(DebugDrawer, Clear),
                                         asCALL_THISCALL));
}

void bindTimer(asIScriptEngine* engine) {
    // Time
    AS_CALL(engine->RegisterObjectMethod(
        "Time", "void Update()", asMETHOD(Time, Update), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Time", "TimeType GetCurrentTime() const",
        asMETHOD(Time, GetCurrentTime), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Time", "TimeType GetElapseTime() const", asMETHOD(Time, GetElapseTime),
        asCALL_THISCALL));

    // Timer
    AS_CALL(engine->RegisterObjectBehaviour("TimerID", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(TimerID, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void SetInterval(TimeType)",
                                         asMETHOD(Timer, SetInterval),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Update(TimeType)",
                                         asMETHOD(Timer, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "void Start()", asMETHOD(Timer, Start), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "void Stop()", asMETHOD(Timer, Stop), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "void Rewind()", asMETHOD(Timer, Rewind), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void SetLoop(int)",
                                         asMETHOD(Timer, SetLoop),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "void Pause()", asMETHOD(Timer, Pause), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "TimeType GetInterval() const", asMETHOD(Timer, GetInterval),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "TimerEventType GetEventType() const",
        asMETHOD(Timer, GetEventType), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Timer", "void SetEventType(TimerEventType)",
        asMETHOD(Timer, SetEventType), asCALL_THISCALL));

    // TimerManager
    AS_CALL(engine->RegisterObjectMethod(
        "TimerManager", "Timer@ Create(TimeType, TimerEventType, int)",
        asMETHOD(TimerManager, Create), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "TimerManager", "void RemoveTimer(TimerID)",
        asMETHOD(TimerManager, RemoveTimer), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "void Clear()",
                                         asMETHOD(TimerManager, Clear),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "TimerManager", "void Update(TimeType)", asMETHOD(TimerManager, Update),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "Timer@ Find(TimerID)",
                                         asMETHOD(TimerManager, Find),
                                         asCALL_THISCALL));
}

void bindTrigger(asIScriptEngine* engine) {
    // TriggerEnterEvent
    AS_CALL(engine->RegisterObjectMethod(
        "TriggerEnterEvent", "TriggerEventType GetType() const",
        asMETHOD(TriggerEnterEvent, GetType), asCALL_THISCALL));

    // TriggerLeaveEvent
    AS_CALL(engine->RegisterObjectMethod(
        "TriggerLeaveEvent", "TriggerEventType GetType() const",
        asMETHOD(TriggerLeaveEvent, GetType), asCALL_THISCALL));

    // TriggerTouchEvent
    AS_CALL(engine->RegisterObjectMethod(
        "TriggerTouchEvent", "TriggerEventType GetType() const",
        asMETHOD(TriggerTouchEvent, GetType), asCALL_THISCALL));

    // Trigger
    AS_CALL(engine->RegisterObjectMethod(
        "Trigger", "const PhysicsActor@ GetActor() const",
        asMETHOD(Trigger, GetActor), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Trigger", "void SetEventType(TriggerEventType)",
        asMETHOD(Trigger, SetEventType), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Trigger", "TriggerEventType GetEventType() const",
        asMETHOD(Trigger, GetEventType), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Trigger", "void EnableTriggerEveryFrameWhenTouch(bool)",
        asMETHOD(Trigger, EnableTriggerEveryFrameWhenTouch), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Trigger", "bool IsTriggerEveryFrameWhenTouch() const",
        asMETHOD(Trigger, IsTriggerEveryFrameWhenTouch), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "void Update()",
                                         asMETHOD(Trigger, Update),
                                         asCALL_THISCALL));

    // TriggerComponentManager
    AS_CALL(engine->RegisterObjectMethod(
        "TriggerComponentManager", "void Update()",
        asMETHOD(TriggerComponentManager, Update), asCALL_THISCALL));
}

void bindContext(asIScriptEngine* engine) {
    // GameContext global instance
    static GameContext* g_game_context = nullptr;
    g_game_context = &GameContext::GetInst();
    AS_CALL(engine->SetDefaultNamespace("TL"));
    // AS_CALL(engine->RegisterGlobalProperty("GameContext@ GAME_CONTEXT",
    //                                        &GAME_CONTEXT));
    AS_CALL(engine->RegisterGlobalProperty(
        "const NullEntity null_entity", const_cast<NullEntity*>(&null_entity)));
    // 必须返回 GameContext*，不能直接绑 GetInst()（返回 GameContext&）。
    // 在 ARM/Android 上“返回引用”与“返回指针”的 ABI 可能不同，会导致脚本拿到的
    // handle 指向错误地址，进而 m_input_manager 等野指针崩溃。
    AS_CALL(engine->RegisterGlobalFunction(
        "GameContext@ GetGameContext()",
        asFUNCTION(+[]() -> GameContext* { return &GameContext::GetInst(); }),
        asCALL_CDECL));

    // Methods inherited from CommonContext
    AS_CALL(engine->RegisterObjectMethod("GameContext", "Entity CreateEntity()",
                                         asMETHOD(GameContext, CreateEntity),
                                         asCALL_THISCALL));

    // Getter methods for public member variables (because they are unique_ptr)
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "Window@ get_m_window() property",
        asFUNCTION(
            +[](GameContext* ctx) -> Window* { return ctx->m_window.get(); }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "Renderer@ get_m_renderer() property",
        asFUNCTION(+[](GameContext* ctx) -> Renderer* {
            return ctx->m_renderer.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "DebugDrawer@ get_m_debug_drawer() property",
        asFUNCTION(+[](GameContext* ctx) -> DebugDrawer* {
            return dynamic_cast<DebugDrawer*>(ctx->m_debug_drawer.get());
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "Time@ get_m_time() property",
        asFUNCTION(
            +[](GameContext* ctx) -> Time* { return ctx->m_time.get(); }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "PhysicsScene@ get_m_physics_scene() property",
        asFUNCTION(+[](GameContext* ctx) -> PhysicsScene* {
            return ctx->m_physics_scene.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "CCTManager@ get_m_cct_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> CCTManager* {
            return ctx->m_cct_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "InputManager@ get_m_input_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> InputManager* {
            return ctx->m_input_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "AssetsManager@ get_m_assets_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> AssetsManager* {
            return ctx->m_assets_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "TransformManager@ get_m_transform_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> TransformManager* {
            return ctx->m_transform_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "SpriteManager@ get_m_sprite_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> SpriteManager* {
            return ctx->m_sprite_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "RelationshipManager@ get_m_relationship_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> RelationshipManager* {
            return ctx->m_relationship_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "LevelManager@ get_m_level_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> LevelManager* {
            return ctx->m_level_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "TimerManager@ get_m_timer_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> TimerManager* {
            return ctx->m_timer_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "AnimationPlayerManager@ get_m_animation_player_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> AnimationPlayerManager* {
            return ctx->m_animation_player_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "GameplayConfigManager@ get_m_gameplay_config_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> GameplayConfigManager* {
            return ctx->m_gameplay_config_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext", "UIComponentManager@ get_m_ui_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> UIComponentManager* {
            return ctx->m_ui_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "TriggerComponentManager@ get_m_trigger_component_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> TriggerComponentManager* {
            return ctx->m_trigger_component_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "TilemapComponentManager@ get_m_tilemap_component_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> TilemapComponentManager* {
            return ctx->m_tilemap_component_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "GameContext",
        "ScriptComponentManager@ get_m_script_manager() property",
        asFUNCTION(+[](GameContext* ctx) -> ScriptComponentManager* {
            return ctx->m_script_component_manager.get();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectProperty("GameContext", "Camera m_camera",
                                           asOFFSET(CommonContext, m_camera)));
}

void bindAssetsManager(asIScriptEngine* engine) {
    // Specialized versions of GetManager() template method (according to
    // conditions in asset_manager.hpp)
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "ImageManager@ GetImageManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> ImageManager* {
            return &mgr->GetManager<Image>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "TilemapManager@ GetTilemapManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> TilemapManager* {
            return &mgr->GetManager<Tilemap>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "AnimationManager@ GetAnimationManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> AnimationManager* {
            return &mgr->GetManager<Animation>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "LevelManager@ GetLevelManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> LevelManager* {
            return &mgr->GetManager<Level>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "PrefabManager@ GetPrefabManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> PrefabManager* {
            return &mgr->GetManager<Prefab>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager", "FontManager@ GetFontManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> FontManager* {
            return &mgr->GetManager<Font>();
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "AssetsManager",
        "ScriptBinaryDataManager@ GetScriptBinaryDataManager()",
        asFUNCTION(+[](AssetsManager* mgr) -> ScriptBinaryDataManager* {
            return &mgr->GetManager<ScriptBinaryData>();
        }),
        asCALL_CDECL_OBJFIRST));
}

void bindMath(asIScriptEngine* engine) {
    AS_CALL(engine->SetDefaultNamespace("TL"));
    bindTVec2Type<float>(engine, "Vec2", "float");
    bindTVec2Type<uint32_t>(engine, "Vec2UI", "uint32");
    AS_CALL(engine->SetDefaultNamespace("TL::Vec2"));
    AS_CALL(
        engine->RegisterGlobalProperty("const Vec2 ZERO", &TVec2<float>::ZERO));
    AS_CALL(engine->RegisterGlobalProperty("const Vec2 X_UNIT",
                                           &TVec2<float>::X_UNIT));
    AS_CALL(engine->RegisterGlobalProperty("const Vec2 Y_UNIT",
                                           &TVec2<float>::Y_UNIT));
    AS_CALL(engine->SetDefaultNamespace("TL::Vec2UI"));
    AS_CALL(engine->RegisterGlobalProperty("const Vec2UI ZERO",
                                           &TVec2<uint32_t>::ZERO));
    AS_CALL(engine->RegisterGlobalProperty("const Vec2UI X_UNIT",
                                           &TVec2<uint32_t>::X_UNIT));
    AS_CALL(engine->RegisterGlobalProperty("const Vec2UI Y_UNIT",
                                           &TVec2<uint32_t>::Y_UNIT));
    AS_CALL(engine->SetDefaultNamespace("TL"));
    bindDegrees(engine);
    bindRadians(engine);
    bindColor(engine);
    bindTransform(engine);
    bindOtherMath(engine);
}

void bindMisc(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterGlobalFunction(
        "float GetZOrderByYSorting(float y, RenderLayer)",
        asFUNCTION(GetZOrderByYSorting), asCALL_CDECL));
}

//-----------------------------------------------------------------------------
// ImGui bindings: common widgets and helpers (namespace ImGui)
//-----------------------------------------------------------------------------
Vec2 ImGui_GetWindowSize() {
    ImVec2 v = ImGui::GetWindowSize();
    return Vec2(v.x, v.y);
}
Vec2 ImGui_GetContentRegionAvail() {
    ImVec2 v = ImGui::GetContentRegionAvail();
    return Vec2(v.x, v.y);
}
Vec2 ImGui_GetCursorScreenPos() {
    ImVec2 v = ImGui::GetCursorScreenPos();
    return Vec2(v.x, v.y);
}
Vec2 ImGui_GetCursorPos() {
    ImVec2 v = ImGui::GetCursorPos();
    return Vec2(v.x, v.y);
}
bool ImGui_InputText(const std::string& label, std::string& value,
                           int max_size) {
    std::vector<char> buf(static_cast<size_t>(max_size) + 1, '\0');
    size_t copy_len = (std::min)(value.size(), static_cast<size_t>(max_size));
    if (copy_len > 0) value.copy(buf.data(), copy_len);
    buf[copy_len] = '\0';
    bool ret = ImGui::InputText(label.c_str(), buf.data(),
                               static_cast<size_t>(max_size));
    if (ret) value = std::string(buf.data());
    return ret;
}

void bindImGui(asIScriptEngine* engine) {
    //----------------------------------------------------------------------
    // ImGui enums (script: ImGui::WindowFlags_None etc.)
    //----------------------------------------------------------------------
    AS_CALL(engine->SetDefaultNamespace(""));

    AS_CALL(engine->RegisterEnum("ImGuiWindowFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "None", ImGuiWindowFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoTitleBar", ImGuiWindowFlags_NoTitleBar));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoResize", ImGuiWindowFlags_NoResize));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoMove", ImGuiWindowFlags_NoMove));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoScrollbar", ImGuiWindowFlags_NoScrollbar));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoCollapse", ImGuiWindowFlags_NoCollapse));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoBackground", ImGuiWindowFlags_NoBackground));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "MenuBar", ImGuiWindowFlags_MenuBar));
    AS_CALL(engine->RegisterEnumValue("ImGuiWindowFlags", "HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar));

    AS_CALL(engine->RegisterEnum("ImGuiChildFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "None", ImGuiChildFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "Borders", ImGuiChildFlags_Borders));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "ResizeX", ImGuiChildFlags_ResizeX));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "ResizeY", ImGuiChildFlags_ResizeY));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "AutoResizeX", ImGuiChildFlags_AutoResizeX));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "AutoResizeY", ImGuiChildFlags_AutoResizeY));
    AS_CALL(engine->RegisterEnumValue("ImGuiChildFlags", "FrameStyle", ImGuiChildFlags_FrameStyle));

    AS_CALL(engine->RegisterEnum("ImGuiTreeNodeFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "None", ImGuiTreeNodeFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "Selected", ImGuiTreeNodeFlags_Selected));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "Framed", ImGuiTreeNodeFlags_Framed));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "AllowOverlap", ImGuiTreeNodeFlags_AllowOverlap));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "Leaf", ImGuiTreeNodeFlags_Leaf));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "Bullet", ImGuiTreeNodeFlags_Bullet));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "FramePadding", ImGuiTreeNodeFlags_FramePadding));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth));
    AS_CALL(engine->RegisterEnumValue("ImGuiTreeNodeFlags", "CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader));

    AS_CALL(engine->RegisterEnum("ImGuiSliderFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiSliderFlags", "None", ImGuiSliderFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiSliderFlags", "Logarithmic", ImGuiSliderFlags_Logarithmic));
    AS_CALL(engine->RegisterEnumValue("ImGuiSliderFlags", "NoRoundToFormat", ImGuiSliderFlags_NoRoundToFormat));
    AS_CALL(engine->RegisterEnumValue("ImGuiSliderFlags", "NoInput", ImGuiSliderFlags_NoInput));
    AS_CALL(engine->RegisterEnumValue("ImGuiSliderFlags", "AlwaysClamp", ImGuiSliderFlags_AlwaysClamp));

    AS_CALL(engine->RegisterEnum("ImGuiComboFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "None", ImGuiComboFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "HeightSmall", ImGuiComboFlags_HeightSmall));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "HeightRegular", ImGuiComboFlags_HeightRegular));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "HeightLarge", ImGuiComboFlags_HeightLarge));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "HeightLargest", ImGuiComboFlags_HeightLargest));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "NoArrowButton", ImGuiComboFlags_NoArrowButton));
    AS_CALL(engine->RegisterEnumValue("ImGuiComboFlags", "NoPreview", ImGuiComboFlags_NoPreview));

    AS_CALL(engine->RegisterEnum("ImGuiSelectableFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "None", ImGuiSelectableFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "NoAutoClosePopups", ImGuiSelectableFlags_NoAutoClosePopups));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "Disabled", ImGuiSelectableFlags_Disabled));
    AS_CALL(engine->RegisterEnumValue("ImGuiSelectableFlags", "AllowOverlap", ImGuiSelectableFlags_AllowOverlap));

    AS_CALL(engine->RegisterEnum("ImGuiTabBarFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabBarFlags", "None", ImGuiTabBarFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabBarFlags", "Reorderable", ImGuiTabBarFlags_Reorderable));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabBarFlags", "AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabBarFlags", "FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabBarFlags", "FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll));

    AS_CALL(engine->RegisterEnum("ImGuiTabItemFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabItemFlags", "None", ImGuiTabItemFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabItemFlags", "UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabItemFlags", "SetSelected", ImGuiTabItemFlags_SetSelected));
    AS_CALL(engine->RegisterEnumValue("ImGuiTabItemFlags", "NoPushId", ImGuiTabItemFlags_NoPushId));

    AS_CALL(engine->RegisterEnum("ImGuiTableFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "None", ImGuiTableFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "Resizable", ImGuiTableFlags_Resizable));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "Reorderable", ImGuiTableFlags_Reorderable));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "Hideable", ImGuiTableFlags_Hideable));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "Sortable", ImGuiTableFlags_Sortable));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "NoSavedSettings", ImGuiTableFlags_NoSavedSettings));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "RowBg", ImGuiTableFlags_RowBg));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersInnerH", ImGuiTableFlags_BordersInnerH));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersOuterH", ImGuiTableFlags_BordersOuterH));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersInnerV", ImGuiTableFlags_BordersInnerV));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersOuterV", ImGuiTableFlags_BordersOuterV));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersH", ImGuiTableFlags_BordersH));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "BordersV", ImGuiTableFlags_BordersV));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "Borders", ImGuiTableFlags_Borders));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "SizingFixedFit", ImGuiTableFlags_SizingFixedFit));
    AS_CALL(engine->RegisterEnumValue("ImGuiTableFlags", "SizingStretchSame", ImGuiTableFlags_SizingStretchSame));

    AS_CALL(engine->RegisterEnum("ImGuiInputTextFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "None", ImGuiInputTextFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "CharsDecimal", ImGuiInputTextFlags_CharsDecimal));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "ReadOnly", ImGuiInputTextFlags_ReadOnly));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "Password", ImGuiInputTextFlags_Password));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll));
    AS_CALL(engine->RegisterEnumValue("ImGuiInputTextFlags", "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue));

    AS_CALL(engine->RegisterEnum("ImGuiPopupFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiPopupFlags", "None", ImGuiPopupFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiPopupFlags", "MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft));
    AS_CALL(engine->RegisterEnumValue("ImGuiPopupFlags", "MouseButtonRight", ImGuiPopupFlags_MouseButtonRight));
    AS_CALL(engine->RegisterEnumValue("ImGuiPopupFlags", "MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle));
    AS_CALL(engine->RegisterEnumValue("ImGuiPopupFlags", "NoReopen", ImGuiPopupFlags_NoReopen));

    AS_CALL(engine->RegisterEnum("ImGuiFocusedFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiFocusedFlags", "None", ImGuiFocusedFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiFocusedFlags", "ChildWindows", ImGuiFocusedFlags_ChildWindows));
    AS_CALL(engine->RegisterEnumValue("ImGuiFocusedFlags", "RootWindow", ImGuiFocusedFlags_RootWindow));
    AS_CALL(engine->RegisterEnumValue("ImGuiFocusedFlags", "AnyWindow", ImGuiFocusedFlags_AnyWindow));
    AS_CALL(engine->RegisterEnumValue("ImGuiFocusedFlags", "RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows));

    AS_CALL(engine->RegisterEnum("ImGuiHoveredFlags"));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "None", ImGuiHoveredFlags_None));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "ChildWindows", ImGuiHoveredFlags_ChildWindows));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "RootWindow", ImGuiHoveredFlags_RootWindow));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "AnyWindow", ImGuiHoveredFlags_AnyWindow));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup));
    AS_CALL(engine->RegisterEnumValue("ImGuiHoveredFlags", "AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled));

    //----------------------------------------------------------------------
    // ImGui Widgets
    //----------------------------------------------------------------------
    AS_CALL(engine->SetDefaultNamespace("ImGui"));

    // Window
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Begin(const string& in name)",
        asFUNCTION(+[](const std::string& name) {
            return ImGui::Begin(name.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Begin(const string& in name, bool& out p_open)",
        asFUNCTION(+[](const std::string& name, bool* p_open) {
            return ImGui::Begin(name.c_str(), p_open);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Begin(const string& in name, bool &out p_open, int flags)",
        asFUNCTION(+[](const std::string& name, bool* p_open, int flags) {
            return ImGui::Begin(name.c_str(), p_open,
                                static_cast<ImGuiWindowFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Begin(const string& in name, int flags)",
        asFUNCTION(+[](const std::string& name, int flags) {
            return ImGui::Begin(name.c_str(), nullptr,
                                static_cast<ImGuiWindowFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void End()", asFUNCTION(ImGui::End),
                                            asCALL_CDECL));

    // Child window
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginChild(const string& in str_id, float size_x = 0, float size_y = 0)",
        asFUNCTION(+[](const std::string& str_id, float sx, float sy) {
            return ImGui::BeginChild(str_id.c_str(), ImVec2(sx, sy));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginChild(const string& in str_id, float size_x, float size_y, int child_flags, int window_flags = 0)",
        asFUNCTION(+[](const std::string& str_id, float sx, float sy,
                       int child_flags, int window_flags) {
            return ImGui::BeginChild(str_id.c_str(), ImVec2(sx, sy),
                                     static_cast<ImGuiChildFlags>(child_flags),
                                     static_cast<ImGuiWindowFlags>(window_flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndChild()",
                                            asFUNCTION(ImGui::EndChild),
                                            asCALL_CDECL));

    // Window utilities
    AS_CALL(engine->RegisterGlobalFunction(
        "void SetNextWindowPos(float x, float y)",
        asFUNCTION(+[](float x, float y) {
            ImGui::SetNextWindowPos(ImVec2(x, y));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void SetNextWindowSize(float x, float y)",
        asFUNCTION(+[](float x, float y) {
            ImGui::SetNextWindowSize(ImVec2(x, y));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("TL::Vec2 GetWindowSize()",
                                            asFUNCTION(ImGui_GetWindowSize),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool IsWindowFocused(int flags = 0)",
                                            asFUNCTION(+[](int flags) {
                                                return ImGui::IsWindowFocused(
                                                    static_cast<ImGuiFocusedFlags>(flags));
                                            }),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool IsWindowHovered(int flags = 0)",
                                            asFUNCTION(+[](int flags) {
                                                return ImGui::IsWindowHovered(
                                                    static_cast<ImGuiHoveredFlags>(flags));
                                            }),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("TL::Vec2 GetContentRegionAvail()",
                                            asFUNCTION(ImGui_GetContentRegionAvail),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("TL::Vec2 GetCursorScreenPos()",
                                            asFUNCTION(ImGui_GetCursorScreenPos),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("TL::Vec2 GetCursorPos()",
                                            asFUNCTION(ImGui_GetCursorPos),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("float GetCursorPosX()",
                                            asFUNCTION(ImGui::GetCursorPosX),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("float GetCursorPosY()",
                                            asFUNCTION(ImGui::GetCursorPosY),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void SetCursorPos(float x, float y)",
        asFUNCTION(+[](float x, float y) { ImGui::SetCursorPos(ImVec2(x, y)); }),
        asCALL_CDECL));

    // Widgets: Text
    AS_CALL(engine->RegisterGlobalFunction(
        "void Text(const string& in text)",
        asFUNCTION(+[](const std::string& text) {
            ImGui::TextUnformatted(text.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void TextDisabled(const string& in text)",
        asFUNCTION(+[](const std::string& text) {
            ImGui::TextDisabled("%s", text.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void TextWrapped(const string& in text)",
        asFUNCTION(+[](const std::string& text) {
            ImGui::TextWrapped("%s", text.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void SeparatorText(const string& in label)",
        asFUNCTION(+[](const std::string& label) {
            ImGui::SeparatorText(label.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void BulletText(const string& in text)",
        asFUNCTION(+[](const std::string& text) {
            ImGui::BulletText("%s", text.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void Bullet()",
                                            asFUNCTION(ImGui::Bullet),
                                            asCALL_CDECL));

    // Widgets: Main
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Button(const string& in label)",
        asFUNCTION(+[](const std::string& label) {
            return ImGui::Button(label.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Button(const string& in label, float size_x, float size_y)",
        asFUNCTION(+[](const std::string& label, float sx, float sy) {
            return ImGui::Button(label.c_str(), ImVec2(sx, sy));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool SmallButton(const string& in label)",
        asFUNCTION(+[](const std::string& label) {
            return ImGui::SmallButton(label.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Checkbox(const string& in label, bool &out v)",
        asFUNCTION(+[](const std::string& label, bool* v) {
            return ImGui::Checkbox(label.c_str(), v);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool RadioButton(const string& in label, bool active)",
        asFUNCTION(+[](const std::string& label, bool active) {
            return ImGui::RadioButton(label.c_str(), active);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool RadioButton(const string& in label, int &out v, int v_button)",
        asFUNCTION(+[](const std::string& label, int* v, int v_button) {
            return ImGui::RadioButton(label.c_str(), v, v_button);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void ProgressBar(float fraction, float size_x = -1, float size_y = 0)",
        asFUNCTION(+[](float fraction, float sx, float sy) {
            ImGui::ProgressBar(fraction, ImVec2(sx, sy), nullptr);
        }),
        asCALL_CDECL));

    // Widgets: Input
    AS_CALL(engine->RegisterGlobalFunction(
        "bool InputText(const string& in label, string &out value, int max_size = 256)",
        asFUNCTION(+[](const std::string& label, std::string& value, int max_size) {
            return ImGui_InputText(label, value, max_size);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool InputText(const string& in label, string &out value, int max_size, int flags)",
        asFUNCTION(+[](const std::string& label, std::string& value, int max_size,
                       int flags) {
            std::vector<char> buf(static_cast<size_t>(max_size) + 1, '\0');
            size_t copy_len = (std::min)(value.size(), static_cast<size_t>(max_size));
            if (copy_len > 0) value.copy(buf.data(), copy_len);
            buf[copy_len] = '\0';
            bool ret = ImGui::InputText(label.c_str(), buf.data(),
                                        static_cast<size_t>(max_size),
                                        static_cast<ImGuiInputTextFlags>(flags));
            if (ret) value = std::string(buf.data());
            return ret;
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool InputInt(const string& in label, int &out v, int step = 1, int step_fast = 100)",
        asFUNCTION(+[](const std::string& label, int* v, int step, int step_fast) {
            return ImGui::InputInt(label.c_str(), v, step, step_fast);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool InputFloat(const string& in label, float &out v, float step = 0, float step_fast = 0, const string& in format = '%.3f')",
        asFUNCTION(+[](const std::string& label, float* v, float step,
                       float step_fast, const std::string& format) {
            return ImGui::InputFloat(label.c_str(), v, step, step_fast,
                                    format.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool InputDouble(const string& in label, double &out v, double step = 0, double step_fast = 0, const string& in format = '%.6f')",
        asFUNCTION(+[](const std::string& label, double* v, double step,
                       double step_fast, const std::string& format) {
            return ImGui::InputDouble(label.c_str(), v, step, step_fast,
                                     format.c_str());
        }),
        asCALL_CDECL));

    // Widgets: Drag
    AS_CALL(engine->RegisterGlobalFunction(
        "bool DragInt(const string& in label, int &out v, float v_speed = 1, int v_min = 0, int v_max = 0, const string& in format = '%d', int flags = 0)",
        asFUNCTION(+[](const std::string& label, int* v, float speed, int vmin,
                       int vmax, const std::string& format, int flags) {
            return ImGui::DragInt(label.c_str(), v, speed, vmin, vmax,
                                 format.c_str(),
                                 static_cast<ImGuiSliderFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool DragFloat(const string& in label, float &out v, float v_speed = 1, float v_min = 0, float v_max = 0, const string& in format = '%.3f', int flags = 0)",
        asFUNCTION(+[](const std::string& label, float* v, float speed, float vmin,
                       float vmax, const std::string& format, int flags) {
            return ImGui::DragFloat(label.c_str(), v, speed, vmin, vmax,
                                  format.c_str(),
                                  static_cast<ImGuiSliderFlags>(flags));
        }),
        asCALL_CDECL));

    // Widgets: Slider
    AS_CALL(engine->RegisterGlobalFunction(
        "bool SliderInt(const string& in label, int &out v, int v_min, int v_max, const string& in format = '%d', int flags = 0)",
        asFUNCTION(+[](const std::string& label, int* v, int vmin, int vmax,
                       const std::string& format, int flags) {
            return ImGui::SliderInt(label.c_str(), v, vmin, vmax,
                                   format.c_str(),
                                   static_cast<ImGuiSliderFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool SliderFloat(const string& in label, float &out v, float v_min, float v_max, const string& in format = '%.3f', int flags = 0)",
        asFUNCTION(+[](const std::string& label, float* v, float vmin, float vmax,
                       const std::string& format, int flags) {
            return ImGui::SliderFloat(label.c_str(), v, vmin, vmax,
                                     format.c_str(),
                                     static_cast<ImGuiSliderFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool SliderAngle(const string& in label, float &out v_rad, float v_degrees_min = -360, float v_degrees_max = 360, const string& in format = '%.0f deg', int flags = 0)",
        asFUNCTION(+[](const std::string& label, float* v, float vmin, float vmax,
                       const std::string& format, int flags) {
            return ImGui::SliderAngle(label.c_str(), v, vmin, vmax,
                                     format.c_str(),
                                     static_cast<ImGuiSliderFlags>(flags));
        }),
        asCALL_CDECL));

    // Widgets: Tree
    AS_CALL(engine->RegisterGlobalFunction(
        "bool TreeNode(const string& in label)",
        asFUNCTION(+[](const std::string& label) {
            return ImGui::TreeNode(label.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool TreeNodeEx(const string& in label, int flags = 0)",
        asFUNCTION(+[](const std::string& label, int flags) {
            return ImGui::TreeNodeEx(label.c_str(),
                                     static_cast<ImGuiTreeNodeFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void TreePop()",
                                            asFUNCTION(ImGui::TreePop),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool CollapsingHeader(const string& in label, int flags = 0)",
        asFUNCTION(+[](const std::string& label, int flags) {
            return ImGui::CollapsingHeader(label.c_str(),
                                           static_cast<ImGuiTreeNodeFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool CollapsingHeader(const string& in label, bool &out p_visible, int flags = 0)",
        asFUNCTION(+[](const std::string& label, bool* p_visible, int flags) {
            return ImGui::CollapsingHeader(label.c_str(), p_visible,
                                           static_cast<ImGuiTreeNodeFlags>(flags));
        }),
        asCALL_CDECL));

    // Widgets: Selectable
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Selectable(const string& in label, bool selected = false)",
        asFUNCTION(+[](const std::string& label, bool selected) {
            return ImGui::Selectable(label.c_str(), selected);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Selectable(const string& in label, bool selected, int flags)",
        asFUNCTION(+[](const std::string& label, bool selected, int flags) {
            return ImGui::Selectable(label.c_str(), selected,
                                     static_cast<ImGuiSelectableFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Selectable(const string& in label, bool &out p_selected)",
        asFUNCTION(+[](const std::string& label, bool* p_selected) {
            return ImGui::Selectable(label.c_str(), p_selected);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool Selectable(const string& in label, bool &out p_selected, int flags)",
        asFUNCTION(+[](const std::string& label, bool* p_selected, int flags) {
            return ImGui::Selectable(label.c_str(), p_selected,
                                     static_cast<ImGuiSelectableFlags>(flags));
        }),
        asCALL_CDECL));

    // Widgets: Combo
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginCombo(const string& in label, const string& in preview_value)",
        asFUNCTION(+[](const std::string& label, const std::string& preview) {
            return ImGui::BeginCombo(label.c_str(), preview.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginCombo(const string& in label, const string& in preview_value, int flags)",
        asFUNCTION(+[](const std::string& label, const std::string& preview,
                       int flags) {
            return ImGui::BeginCombo(label.c_str(), preview.c_str(),
                                     static_cast<ImGuiComboFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndCombo()",
                                            asFUNCTION(ImGui::EndCombo),
                                            asCALL_CDECL));

    // Layout
    AS_CALL(engine->RegisterGlobalFunction(
        "void SameLine(float offset_from_start_x = 0, float spacing = -1)",
        asFUNCTION(+[](float offset, float spacing) {
            ImGui::SameLine(offset, spacing);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void NewLine()",
                                            asFUNCTION(ImGui::NewLine),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void Separator()",
                                            asFUNCTION(ImGui::Separator),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void Spacing()",
                                            asFUNCTION(ImGui::Spacing),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void Dummy(float size_x, float size_y)",
        asFUNCTION(+[](float x, float y) {
            ImGui::Dummy(ImVec2(x, y));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void Indent(float indent_w = 0)",
        asFUNCTION(ImGui::Indent), asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void Unindent(float indent_w = 0)",
        asFUNCTION(ImGui::Unindent), asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void BeginGroup()",
                                            asFUNCTION(ImGui::BeginGroup),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndGroup()",
                                            asFUNCTION(ImGui::EndGroup),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void AlignTextToFramePadding()",
                                            asFUNCTION(ImGui::AlignTextToFramePadding),
                                            asCALL_CDECL));

    // ID stack
    AS_CALL(engine->RegisterGlobalFunction(
        "void PushID(const string& in str_id)",
        asFUNCTION(+[](const std::string& str_id) {
            ImGui::PushID(str_id.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void PushID(int int_id)",
        asFUNCTION(+[](int id) { ImGui::PushID(id); }), asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void PopID()",
                                            asFUNCTION(ImGui::PopID),
                                            asCALL_CDECL));

    // State: Disabled
    AS_CALL(engine->RegisterGlobalFunction("void BeginDisabled(bool disabled = true)",
                                            asFUNCTION(ImGui::BeginDisabled),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndDisabled()",
                                            asFUNCTION(ImGui::EndDisabled),
                                            asCALL_CDECL));

    // Menu
    AS_CALL(engine->RegisterGlobalFunction("bool BeginMenuBar()",
                                            asFUNCTION(ImGui::BeginMenuBar),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndMenuBar()",
                                            asFUNCTION(ImGui::EndMenuBar),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginMainMenuBar()",
        asFUNCTION(ImGui::BeginMainMenuBar), asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndMainMenuBar()",
                                            asFUNCTION(ImGui::EndMainMenuBar),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginMenu(const string& in label, bool enabled = true)",
        asFUNCTION(+[](const std::string& label, bool enabled) {
            return ImGui::BeginMenu(label.c_str(), enabled);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndMenu()",
                                            asFUNCTION(ImGui::EndMenu),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool MenuItem(const string& in label, const string& in shortcut = '', bool selected = false, bool enabled = true)",
        asFUNCTION(+[](const std::string& label, const std::string& shortcut,
                       bool selected, bool enabled) {
            return ImGui::MenuItem(label.c_str(),
                                  shortcut.empty() ? nullptr : shortcut.c_str(),
                                  selected, enabled);
        }),
        asCALL_CDECL));

    // Popup
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginPopup(const string& in str_id)",
        asFUNCTION(+[](const std::string& str_id) {
            return ImGui::BeginPopup(str_id.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginPopup(const string& in str_id, int flags)",
        asFUNCTION(+[](const std::string& str_id, int flags) {
            return ImGui::BeginPopup(str_id.c_str(),
                                    static_cast<ImGuiWindowFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginPopupModal(const string& in name, bool &out p_open)",
        asFUNCTION(+[](const std::string& name, bool* p_open) {
            return ImGui::BeginPopupModal(name.c_str(), p_open);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginPopupModal(const string& in name, bool &out p_open, int flags)",
        asFUNCTION(+[](const std::string& name, bool* p_open, int flags) {
            return ImGui::BeginPopupModal(name.c_str(), p_open,
                                         static_cast<ImGuiWindowFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndPopup()",
                                            asFUNCTION(ImGui::EndPopup),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void OpenPopup(const string& in str_id)",
        asFUNCTION(+[](const std::string& str_id) {
            ImGui::OpenPopup(str_id.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void OpenPopup(const string& in str_id, int popup_flags)",
        asFUNCTION(+[](const std::string& str_id, int popup_flags) {
            ImGui::OpenPopup(str_id.c_str(),
                             static_cast<ImGuiPopupFlags>(popup_flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void CloseCurrentPopup()",
                                            asFUNCTION(ImGui::CloseCurrentPopup),
                                            asCALL_CDECL));

    // Tab bar
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTabBar(const string& in str_id)",
        asFUNCTION(+[](const std::string& str_id) {
            return ImGui::BeginTabBar(str_id.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTabBar(const string& in str_id, int flags)",
        asFUNCTION(+[](const std::string& str_id, int flags) {
            return ImGui::BeginTabBar(str_id.c_str(),
                                      static_cast<ImGuiTabBarFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndTabBar()",
                                            asFUNCTION(ImGui::EndTabBar),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTabItem(const string& in label, bool &out p_open)",
        asFUNCTION(+[](const std::string& label, bool* p_open) {
            return ImGui::BeginTabItem(label.c_str(), p_open);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTabItem(const string& in label, bool &out p_open, int flags)",
        asFUNCTION(+[](const std::string& label, bool* p_open, int flags) {
            return ImGui::BeginTabItem(label.c_str(), p_open,
                                       static_cast<ImGuiTabItemFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTabItem(const string& in label)",
        asFUNCTION(+[](const std::string& label) {
            return ImGui::BeginTabItem(label.c_str());
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndTabItem()",
                                            asFUNCTION(ImGui::EndTabItem),
                                            asCALL_CDECL));

    // Table
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTable(const string& in str_id, int columns)",
        asFUNCTION(+[](const std::string& str_id, int columns) {
            return ImGui::BeginTable(str_id.c_str(), columns);
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "bool BeginTable(const string& in str_id, int columns, int flags)",
        asFUNCTION(+[](const std::string& str_id, int columns, int flags) {
            return ImGui::BeginTable(str_id.c_str(), columns,
                                     static_cast<ImGuiTableFlags>(flags));
        }),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void EndTable()",
                                            asFUNCTION(ImGui::EndTable),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void TableNextRow()",
                                            asFUNCTION(ImGui::TableNextRow),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool TableNextColumn()",
                                            asFUNCTION(ImGui::TableNextColumn),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool TableSetColumnIndex(int column_index)",
                                            asFUNCTION(ImGui::TableSetColumnIndex),
                                            asCALL_CDECL));

    // Item queries
    AS_CALL(engine->RegisterGlobalFunction("bool IsItemClicked()",
                                            asFUNCTION(ImGui::IsItemClicked),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool IsItemHovered()",
                                            asFUNCTION(ImGui::IsItemHovered),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool IsItemActive()",
                                            asFUNCTION(ImGui::IsItemActive),
                                            asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("bool IsItemVisible()",
                                            asFUNCTION(ImGui::IsItemVisible),
                                            asCALL_CDECL));

    AS_CALL(engine->SetDefaultNamespace("TL"));
}

void bindAllTypes(asIScriptEngine* engine) {
    // Optional<T> (CppOptional) methods
    bindOptionalType(engine);
    // Flags<T> (CppFlags) template methods
    bindFlagsType(engine);
    // Handle<T> (CppHandle) template methods
    bindHandleType(engine);
    // bind schema generate types
    BindSchema(engine);
    // Then register all methods, properties, etc. (actual binding)
    bindMath(engine);
    bindPath(engine);
    bindEntity(engine);
    bindLevel(engine);
    bindImage(engine);
    bindFont(engine);
    bindAnimationManager(engine);
    bindAnimationPlayer(engine);
    bindGameplayConfigManager(engine);
    bindLevelManager(engine);
    bindPrefabManager(engine);
    bindScriptBinaryDataManager(engine);
    bindSprite(engine);
    bindTilemap(engine);
    bindUI(engine);
    bindPhysics(engine);
    bindCharacterController(engine);
    bindCollisionGroup(engine);
    bindRelationship(engine);
    bindWindow(engine);
    bindRenderer(engine);
    bindDebugDrawer(engine);
    bindCamera(engine);
    bindTimer(engine);
    bindTrigger(engine);
    bindInputManager(engine);
    bindContext(engine);
    bindAssetsManager(engine);
    bindMisc(engine);
    bindImGui(engine);
}

void BindTLModule(asIScriptEngine* engine) {
    TL_RETURN_IF_NULL_WITH_LOG(engine, LOGE, "angel script engine is null!");

    AS_CALL(engine->SetDefaultNamespace("TL"));

    AS_CALL(engine->RegisterGlobalFunction(
        "void Log(const string)", asFUNCTION(AngelScriptLog<std::string>),
        asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void Log(int)", asFUNCTION(AngelScriptLog<int>), asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction(
        "void Log(float)", asFUNCTION(AngelScriptLog<float>), asCALL_CDECL));

    // Register all types first (forward binding)
    registerAllTypes(engine);
    bindAllTypes(engine);

    AS_CALL(engine->SetDefaultNamespace(""));
}
