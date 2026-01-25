#pragma once
#include "engine/script_binding.hpp"
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/entity.hpp"
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/level.hpp"
#include "engine/math.hpp"
#include "engine/physics.hpp"
#include "engine/relationship.hpp"
#include "engine/renderer.hpp"
#include "engine/script_macros.hpp"
#include "engine/sprite.hpp"
#include "engine/text.hpp"
#include "engine/tilemap.hpp"
#include "engine/timer.hpp"
#include "engine/trigger.hpp"
#include "engine/ui.hpp"
#include "engine/window.hpp"
#include "schema/prefab.hpp"

template <typename T>
void AngelScriptLog(T msg) {
    LOGI("[Script]: {}", msg);
}

// Template function to register Flags type
template <typename EnumType>
void registerFlagsType(asIScriptEngine* engine, const char* typeName) {
    AS_CALL(engine->RegisterObjectType(typeName, sizeof(Flags<EnumType>),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Flags<EnumType>>()));
}

// Template function to bind Flags type methods
template <typename EnumType>
void bindFlagsType(asIScriptEngine* engine, const char* typeName) {
    // Constructors
    AS_CALL(engine->RegisterObjectBehaviour(typeName, asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Flags<EnumType>, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(typeName, asBEHAVE_CONSTRUCT,
                                            "void f(int)",
                                            asFUNCTION(+[](void* mem, int value) {
                                                new(mem) Flags<EnumType>(static_cast<typename Flags<EnumType>::underlying_type>(value));
                                            }),
                                            asCALL_CDECL_OBJFIRST));
    
    // Value() method
    AS_CALL(engine->RegisterObjectMethod(typeName, "int Value() const",
                                         asMETHOD(Flags<EnumType>, Value),
                                         asCALL_THISCALL));
    
    // Operator |=
    AS_CALL(engine->RegisterObjectMethod(typeName, "void opOrAssign(int)",
                                         asFUNCTION(+[](Flags<EnumType>* f, typename Flags<EnumType>::underlying_type v) {
                                             *f |= static_cast<EnumType>(v);
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    
    // Operator &=
    AS_CALL(engine->RegisterObjectMethod(typeName, "void opAndAssign(int)",
                                         asFUNCTION(+[](Flags<EnumType>* f, typename Flags<EnumType>::underlying_type v) {
                                             *f &= static_cast<EnumType>(v);
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    
    // Remove method
    AS_CALL(engine->RegisterObjectMethod(typeName, "void Remove(int)",
                                         asFUNCTION(+[](Flags<EnumType>* f, typename Flags<EnumType>::underlying_type v) {
                                             f->Remove(static_cast<EnumType>(v));
                                         }),
                                         asCALL_CDECL_OBJFIRST));
}

// Forward binding: register all types
void registerAllTypes(asIScriptEngine* engine) {
    // Math types
    AS_CALL(engine->RegisterObjectType("Vec2", sizeof(Vec2),
                                       asOBJ_VALUE | asOBJ_POD |
                                           asOBJ_APP_CLASS_ALLFLOATS |
                                           asGetTypeTraits<Vec2>()));
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
    AS_CALL(engine->RegisterObjectType("Transform", sizeof(Transform),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Transform>()));
    
    // Entity and Level
    AS_CALL(engine->RegisterObjectType("Entity", sizeof(Entity),
                                       asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE));
    AS_CALL(engine->RegisterObjectType("Level", 0, asOBJ_REF));
    
    // Image
    AS_CALL(engine->RegisterObjectType("Image", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("ImageHandle", sizeof(ImageHandle),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<ImageHandle>()));
    AS_CALL(engine->RegisterObjectType("ImageManager", 0, asOBJ_REF));
    
    // Font
    AS_CALL(engine->RegisterObjectType("Font", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("FontHandle", sizeof(FontHandle),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<FontHandle>()));
    AS_CALL(engine->RegisterObjectType("FontManager", 0, asOBJ_REF));
    
    // Sprite
    AS_CALL(engine->RegisterObjectType("SpriteManager", 0, asOBJ_REF));
    
    // Tilemap
    AS_CALL(engine->RegisterObjectType("Tile", sizeof(Tile),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Tile>()));
    AS_CALL(engine->RegisterObjectType("Tilemap", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("TilemapHandle", sizeof(TilemapHandle),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TilemapHandle>()));
    AS_CALL(engine->RegisterObjectType("TilemapManager", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("TilemapComponent", sizeof(TilemapComponent),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TilemapComponent>()));
    AS_CALL(engine->RegisterObjectType("TilemapComponentManager", 0, asOBJ_REF));
    
    // UI
    AS_CALL(engine->RegisterObjectType("UIText", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("UIWidget", sizeof(UIWidget),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<UIWidget>()));
    AS_CALL(engine->RegisterObjectType("UIComponentManager", 0, asOBJ_REF));
    
    // Physics
    AS_CALL(engine->RegisterObjectType("HitResult", sizeof(HitResult),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<HitResult>()));
    AS_CALL(engine->RegisterObjectType("SweepResult", sizeof(SweepResult),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<SweepResult>()));
    AS_CALL(engine->RegisterObjectType("OverlapResult", sizeof(OverlapResult),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<OverlapResult>()));
    AS_CALL(engine->RegisterObjectType("PhysicsShape", sizeof(PhysicsShape),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<PhysicsShape>()));
    AS_CALL(engine->RegisterObjectType("PhysicsActor", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("PhysicsScene", 0, asOBJ_REF));
    
    // Relationship
    AS_CALL(engine->RegisterObjectType("RelationshipManager", 0, asOBJ_REF));
    
    // Window and Renderer
    AS_CALL(engine->RegisterObjectType("Window", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("Image9Grid", sizeof(Image9Grid),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Image9Grid>()));
    AS_CALL(engine->RegisterObjectType("Renderer", 0, asOBJ_REF));
    
    // Timer
    AS_CALL(engine->RegisterObjectType("Time", 0, asOBJ_REF));
    AS_CALL(engine->RegisterObjectType("Timer", sizeof(Timer),
                                       asOBJ_VALUE | asGetTypeTraits<Timer>()));
    AS_CALL(engine->RegisterObjectType("TimerManager", 0, asOBJ_REF));
    
    // Trigger
    AS_CALL(engine->RegisterObjectType("TriggerEnterEvent", sizeof(TriggerEnterEvent),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerEnterEvent>()));
    AS_CALL(engine->RegisterObjectType("TriggerLeaveEvent", sizeof(TriggerLeaveEvent),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerLeaveEvent>()));
    AS_CALL(engine->RegisterObjectType("TriggerTouchEvent", sizeof(TriggerTouchEvent),
                                       asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<TriggerTouchEvent>()));
    AS_CALL(engine->RegisterObjectType("Trigger", sizeof(Trigger),
                                       asOBJ_VALUE | asGetTypeTraits<Trigger>()));
    AS_CALL(engine->RegisterObjectType("TriggerComponentManager", 0, asOBJ_REF));
    
    // Context
    AS_CALL(engine->RegisterObjectType("GameContext", 0, asOBJ_REF));
    
    // AssetsManager
    AS_CALL(engine->RegisterObjectType("AssetsManager", 0, asOBJ_REF));
    
    // Flags types (concrete types for template Flags<T>)
    registerFlagsType<HitType>(engine, "HitTypeFlags");
    registerFlagsType<UIAnchor>(engine, "UIAnchorFlags");
    registerFlagsType<Flip>(engine, "FlipFlags");
}

void bindVec2(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectBehaviour(
        "Vec2", asBEHAVE_CONSTRUCT, "void f(float x, float y)",
        WRAP_CON(Vec2, (float, float)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("Vec2",
                                         "float Dot(const Vec2& in) const",
                                         asMETHOD(Vec2, Dot), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Vec2", "float Length() const",
                                         asMETHOD(Vec2, Length),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Vec2", "float LengthSquared() const",
                                         asMETHOD(Vec2, LengthSquared),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Vec2", "Vec2 Normalize() const",
                                         asMETHOD(Vec2, Normalize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "bool opEquals(const Vec2&in) const",
        asMETHODPR(Vec2, operator==, (const Vec2&) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2& opAddAssign(const Vec2& in)",
        asMETHODPR(Vec2, operator+=, (const Vec2&), Vec2&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2& opSubAssign(const Vec2& in)",
        asMETHODPR(Vec2, operator-=, (const Vec2&), Vec2&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2& opMulAssign(const Vec2& in)",
        asMETHODPR(Vec2, operator*=, (const Vec2&), Vec2&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2& opDivAssign(const Vec2& in)",
        asMETHODPR(Vec2, operator/=, (const Vec2&), Vec2&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Vec2", "Vec2 opAdd(const Vec2& in)",
                                         asMETHOD(Vec2, operator+),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Vec2", "Vec2 opSub(const Vec2& in)",
                                         asMETHOD(Vec2, operator-),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2 opMul(const Vec2& in)",
        asMETHODPR_CONST(Vec2, operator*, (const Vec2&), Vec2),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2 opDiv(const Vec2& in)",
        asMETHODPR_CONST(Vec2, operator/, (const Vec2&), Vec2),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Vec2", "Vec2 opMul_r(float)",
        asFUNCTIONPR(operator*, (float, const Vec2&), Vec2),
        asCALL_CDECL_OBJLAST));

    AS_CALL(
        engine->RegisterObjectProperty("Vec2", "float x", asOFFSET(Vec2, x)));
    AS_CALL(
        engine->RegisterObjectProperty("Vec2", "float y", asOFFSET(Vec2, y)));
    AS_CALL(
        engine->RegisterObjectProperty("Vec2", "float w", asOFFSET(Vec2, w)));
    AS_CALL(
        engine->RegisterObjectProperty("Vec2", "float h", asOFFSET(Vec2, h)));
}

void bindDegrees(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectBehaviour("Degrees", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Degrees, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Degrees", asBEHAVE_CONSTRUCT, "void f(float)",
        WRAP_CON(Degrees, (float)), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("Degrees", "float Value() const",
                                         asMETHOD(Degrees, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "bool opEquals(const Degrees&in) const",
        asMETHODPR(Degrees, operator==, (Degrees) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "int opCmp(const Degrees&in) const",
        asFUNCTION(+[](Degrees a, Degrees b) {
            return a.Value() < b.Value() ? -1 : a.Value() > b.Value() ? 1 : 0;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opAddAssign(const Degrees& in)",
        asMETHODPR(Degrees, operator+=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opSubAssign(const Degrees& in)",
        asMETHODPR(Degrees, operator-=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opMulAssign(const Degrees& in)",
        asMETHODPR(Degrees, operator*=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees& opDivAssign(const Degrees& in)",
        asMETHODPR(Degrees, operator/=, (Degrees), Degrees&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opAdd(const Degrees& in)",
        asMETHOD(Degrees, operator+), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opSub(const Degrees& in)",
        asMETHOD(Degrees, operator-), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opMul(const Degrees& in)",
        asMETHODPR_CONST(Degrees, operator*, (Degrees), Degrees),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Degrees", "Degrees opDiv(const Degrees& in)",
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
    AS_CALL(engine->RegisterObjectMethod("Radians", "float Value() const",
                                         asMETHOD(Radians, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "bool opEquals(const Radians&in) const",
        asMETHODPR(Radians, operator==, (Radians) const, bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "int opCmp(const Radians&in) const",
        asFUNCTION(+[](Radians a, Radians b) {
            return a.Value() < b.Value() ? -1 : a.Value() > b.Value() ? 1 : 0;
        }),
        asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opAddAssign(const Radians& in)",
        asMETHODPR(Radians, operator+=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opSubAssign(const Radians& in)",
        asMETHODPR(Radians, operator-=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opMulAssign(const Radians& in)",
        asMETHODPR(Radians, operator*=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians& opDivAssign(const Radians& in)",
        asMETHODPR(Radians, operator/=, (Radians), Radians&), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opAdd(const Radians& in)",
        asMETHOD(Radians, operator+), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opSub(const Radians& in)",
        asMETHOD(Radians, operator-), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opMul(const Radians& in)",
        asMETHODPR_CONST(Radians, operator*, (Radians), Radians),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Radians", "Radians opDiv(const Radians& in)",
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
}

void bindOtherMath(asIScriptEngine* engine) {
    engine->RegisterGlobalFunction("Vec2 Rotate(const Vec2& in, Degrees)",
                                   asFUNCTION(Rotate), asCALL_CDECL);
    engine->RegisterGlobalFunction(
        "Radians GetAngle(const Vec2& in norm_a, const Vec2& in norm_b)",
        asFUNCTION(GetAngle), asCALL_CDECL);
}

void bindEntity(asIScriptEngine* engine) {
    // Entity is an enum class, actually uint32_t
    AS_CALL(engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(Entity, ()),
                                            asCALL_GENERIC));
}

void bindLevel(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("Level", "void Initialize()",
                                         asMETHOD(Level, Initialize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "void OnEnter()",
                                         asMETHOD(Level, OnEnter),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "void OnQuit()",
                                         asMETHOD(Level, OnQuit),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "void PoseUpdate()",
                                         asMETHOD(Level, PoseUpdate),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "bool IsInited() const",
                                         asMETHOD(Level, IsInited),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "Entity Instantiate(PrefabHandle@)",
                                         asMETHOD(Level, Instantiate),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "void RemoveEntity(Entity)",
                                         asMETHOD(Level, RemoveEntity),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "Entity GetRootEntity() const",
                                         asMETHOD(Level, GetRootEntity),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Level", "Entity GetUIRootEntity() const",
                                         asMETHOD(Level, GetUIRootEntity),
                                         asCALL_THISCALL));
}

void bindImage(asIScriptEngine* engine) {
    // Image methods
    AS_CALL(engine->RegisterObjectMethod("Image", "Vec2 GetSize() const",
                                         asMETHOD(Image, GetSize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Image", "void ChangeColorMask(const Color& in)",
                                         asMETHOD(Image, ChangeColorMask),
                                         asCALL_THISCALL));
    
    // ImageHandle
    AS_CALL(engine->RegisterObjectBehaviour("ImageHandle", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(ImageHandle, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("ImageHandle", "bool opEquals(const ImageHandle&in)",
                                         asMETHODPR(ImageHandle, operator==, (const ImageHandle&), bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("ImageHandle", "bool opNotEquals(const ImageHandle&in)",
                                         asMETHODPR(ImageHandle, operator!=, (const ImageHandle&), bool),
                                         asCALL_THISCALL));
    // Common Handle methods
    AS_CALL(engine->RegisterObjectMethod("ImageHandle", "Image@ opImplCast()",
                                         asFUNCTION(+[](ImageHandle& h) -> Image* { return h.Get(); }),
                                         asCALL_CDECL_OBJLAST));
    
    // ImageManager
    // Wrapper function: convert string to Path
    AS_CALL(engine->RegisterObjectMethod("ImageManager", "ImageHandle Load(const string& in, bool)",
                                         asFUNCTION(+[](ImageManager* mgr, const std::string& filename, bool force) -> ImageHandle {
                                             return mgr->Load(Path(filename), force);
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
    
    // FontHandle
    AS_CALL(engine->RegisterObjectBehaviour("FontHandle", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(FontHandle, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("FontHandle", "bool opEquals(const FontHandle&in)",
                                         asMETHODPR(FontHandle, operator==, (const FontHandle&), bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("FontHandle", "bool opNotEquals(const FontHandle&in)",
                                         asMETHODPR(FontHandle, operator!=, (const FontHandle&), bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("FontHandle", "Font@ opImplCast()",
                                         asFUNCTION(+[](FontHandle& h) -> Font* { return h.Get(); }),
                                         asCALL_CDECL_OBJLAST));
    
    // FontManager
    AS_CALL(engine->RegisterObjectMethod("FontManager", "FontHandle Load(const string& in, bool)",
                                         asFUNCTION(+[](FontManager* mgr, const std::string& filename, bool force) -> FontHandle {
                                             return mgr->Load(Path(filename), force);
                                         }),
                                         asCALL_CDECL_OBJFIRST));
}

void bindSprite(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("SpriteManager", "void Update()",
                                         asMETHOD(SpriteManager, Update),
                                         asCALL_THISCALL));
}

void bindTilemap(asIScriptEngine* engine) {
    // Tile structure
    AS_CALL(engine->RegisterObjectProperty("Tile", "ImageHandle m_image",
                                          asOFFSET(Tile, m_image)));
    AS_CALL(engine->RegisterObjectProperty("Tile", "Region m_region",
                                          asOFFSET(Tile, m_region)));
    AS_CALL(engine->RegisterObjectProperty("Tile", "uint32 m_id",
                                          asOFFSET(Tile, m_id)));
    
    // Tilemap
    AS_CALL(engine->RegisterObjectMethod("Tilemap", "const Vec2& GetTileSize() const",
                                         asMETHOD(Tilemap, GetTileSize),
                                         asCALL_THISCALL));
    
    // TilemapHandle
    AS_CALL(engine->RegisterObjectBehaviour("TilemapHandle", asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(TilemapHandle, ()),
                                            asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectMethod("TilemapHandle", "bool opEquals(const TilemapHandle&in)",
                                         asMETHODPR(TilemapHandle, operator==, (const TilemapHandle&), bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TilemapHandle", "bool opNotEquals(const TilemapHandle&in)",
                                         asMETHODPR(TilemapHandle, operator!=, (const TilemapHandle&), bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TilemapHandle", "Tilemap@ opImplCast()",
                                         asFUNCTION(+[](TilemapHandle& h) -> Tilemap* { return h.Get(); }),
                                         asCALL_CDECL_OBJLAST));
    
    // TilemapManager
    AS_CALL(engine->RegisterObjectMethod("TilemapManager", "TilemapHandle Load(const string& in, bool)",
                                         asFUNCTION(+[](TilemapManager* mgr, const std::string& filename, bool force) -> TilemapHandle {
                                             return mgr->Load(Path(filename), force);
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    
    // TilemapComponent
    AS_CALL(engine->RegisterObjectMethod("TilemapComponent", "TilemapHandle GetHandle() const",
                                         asMETHOD(TilemapComponent, GetHandle),
                                         asCALL_THISCALL));
    
    // TilemapComponentManager
    AS_CALL(engine->RegisterObjectMethod("TilemapComponentManager", "void Update()",
                                         asMETHOD(TilemapComponentManager, Update),
                                         asCALL_THISCALL));
}

void bindUI(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectProperty("UIText", "UITextAlign m_align",
                                          asOFFSET(UIText, m_align)));
    AS_CALL(engine->RegisterObjectProperty("UIText", "bool m_resize_by_text",
                                          asOFFSET(UIText, m_resize_by_text)));
    AS_CALL(engine->RegisterObjectProperty("UIText", "Color m_color",
                                          asOFFSET(UIText, m_color)));
    AS_CALL(engine->RegisterObjectMethod("UIText", "void SetFont(FontHandle)",
                                         asMETHOD(UIText, SetFont),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIText", "void ChangeText(const string& in)",
                                         asMETHOD(UIText, ChangeText),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIText", "void ChangeTextPt(uint32)",
                                         asMETHOD(UIText, ChangeTextPt),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIText", "Vec2 GetTextImageSize() const",
                                         asMETHOD(UIText, GetTextImageSize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIText", "const string& GetText() const",
                                         asMETHOD(UIText, GetText),
                                         asCALL_THISCALL));
    
    // UIWidget
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "UIAnchorFlags m_anchor",
                                          asOFFSET(UIWidget, m_anchor)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_use_clip",
                                          asOFFSET(UIWidget, m_use_clip)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_disabled",
                                          asOFFSET(UIWidget, m_disabled)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_selected",
                                          asOFFSET(UIWidget, m_selected)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "bool m_can_be_selected",
                                          asOFFSET(UIWidget, m_can_be_selected)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "UIState m_state",
                                          asOFFSET(UIWidget, m_state)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "Vec2 m_margin",
                                          asOFFSET(UIWidget, m_margin)));
    AS_CALL(engine->RegisterObjectProperty("UIWidget", "Vec2 m_padding",
                                          asOFFSET(UIWidget, m_padding)));
    
    // UIComponentManager
    AS_CALL(engine->RegisterObjectMethod("UIComponentManager", "void Update()",
                                         asMETHOD(UIComponentManager, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIComponentManager", "void Render()",
                                         asMETHOD(UIComponentManager, Render),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("UIComponentManager", "void HandleEvent()",
                                         asMETHOD(UIComponentManager, HandleEvent),
                                         asCALL_THISCALL));
}

void bindPhysics(asIScriptEngine* engine) {
    // HitType enum
    AS_CALL(engine->RegisterEnum("HitType"));
    AS_CALL(engine->RegisterEnumValue("HitType", "None", static_cast<int>(HitType::None)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Left", static_cast<int>(HitType::Left)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Right", static_cast<int>(HitType::Right)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Top", static_cast<int>(HitType::Top)));
    AS_CALL(engine->RegisterEnumValue("HitType", "Bottom", static_cast<int>(HitType::Bottom)));
    
    // PhysicsShapeType enum
    AS_CALL(engine->RegisterEnum("PhysicsShapeType"));
    AS_CALL(engine->RegisterEnumValue("PhysicsShapeType", "Unknown", static_cast<int>(PhysicsShapeType::Unknown)));
    AS_CALL(engine->RegisterEnumValue("PhysicsShapeType", "Rect", static_cast<int>(PhysicsShapeType::Rect)));
    AS_CALL(engine->RegisterEnumValue("PhysicsShapeType", "Circle", static_cast<int>(PhysicsShapeType::Circle)));
    
    // HitResult
    AS_CALL(engine->RegisterObjectProperty("HitResult", "float m_t",
                                          asOFFSET(HitResult, m_t)));
    AS_CALL(engine->RegisterObjectProperty("HitResult", "HitTypeFlags m_flags",
                                          asOFFSET(HitResult, m_flags)));
    AS_CALL(engine->RegisterObjectProperty("HitResult", "Vec2 m_normal",
                                          asOFFSET(HitResult, m_normal)));
    AS_CALL(engine->RegisterObjectProperty("HitResult", "bool m_is_initial_overlap",
                                          asOFFSET(HitResult, m_is_initial_overlap)));
    
    // SweepResult
    AS_CALL(engine->RegisterObjectProperty("SweepResult", "Entity m_entity",
                                          asOFFSET(SweepResult, m_entity)));
    
    // OverlapResult
    AS_CALL(engine->RegisterObjectProperty("OverlapResult", "Entity m_dst_entity",
                                          asOFFSET(OverlapResult, m_dst_entity)));
    
    // PhysicsShape
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "const Rect@ AsRect() const",
                                         asMETHOD(PhysicsShape, AsRect),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "const Circle@ AsCircle() const",
                                         asMETHOD(PhysicsShape, AsCircle),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "PhysicsShapeType GetType() const",
                                         asMETHOD(PhysicsShape, GetType),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "const Vec2& GetPosition() const",
                                         asMETHOD(PhysicsShape, GetPosition),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "void MoveTo(const Vec2& in)",
                                         asMETHOD(PhysicsShape, MoveTo),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsShape", "void Move(const Vec2& in)",
                                         asMETHOD(PhysicsShape, Move),
                                         asCALL_THISCALL));
    
    // PhysicsActor
    AS_CALL(engine->RegisterObjectMethod("PhysicsActor", "const PhysicsShape& GetShape() const",
                                         asMETHOD(PhysicsActor, GetShape),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsActor", "const Vec2& GetPosition() const",
                                         asMETHOD(PhysicsActor, GetPosition),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsActor", "Entity GetEntity() const",
                                         asMETHOD(PhysicsActor, GetEntity),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsActor", "void MoveTo(const Vec2& in)",
                                         asMETHOD(PhysicsActor, MoveTo),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsActor", "void Move(const Vec2& in)",
                                         asMETHOD(PhysicsActor, Move),
                                         asCALL_THISCALL));
    
    // PhysicsScene
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "PhysicsActor@ CreateActor(Entity, const Rect& in)",
                                         asMETHODPR(PhysicsScene, CreateActor, (Entity, const Rect&), PhysicsActor*),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "PhysicsActor@ CreateActor(Entity, const Circle& in)",
                                         asMETHODPR(PhysicsScene, CreateActor, (Entity, const Circle&), PhysicsActor*),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "void RemoveActor(PhysicsActor@)",
                                         asMETHOD(PhysicsScene, RemoveActor),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "uint32 Sweep(const PhysicsActor& in, const Vec2& in, float, SweepResult[]& out, uint32)",
                                         asMETHODPR(PhysicsScene, Sweep, (const PhysicsActor&, const Vec2&, float, SweepResult*, size_t), uint32_t),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "uint32 Overlap(const PhysicsActor& in, OverlapResult[]& out, uint32)",
                                         asMETHODPR(PhysicsScene, Overlap, (const PhysicsActor&, OverlapResult*, size_t), uint32_t),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "bool Overlap(const PhysicsActor& in, const PhysicsActor& in) const",
                                         asMETHODPR(PhysicsScene, Overlap, (const PhysicsActor&, const PhysicsActor&) const, bool),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "bool IsEnableDebugDraw() const",
                                         asMETHOD(PhysicsScene, IsEnableDebugDraw),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "void ToggleDebugDraw()",
                                         asMETHOD(PhysicsScene, ToggleDebugDraw),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("PhysicsScene", "void RenderDebug() const",
                                         asMETHOD(PhysicsScene, RenderDebug),
                                         asCALL_THISCALL));
}

void bindRelationship(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("RelationshipManager", "void Update()",
                                         asMETHOD(RelationshipManager, Update),
                                         asCALL_THISCALL));
}

void bindWindow(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("Window", "Vec2 GetWindowSize() const",
                                         asMETHOD(Window, GetWindowSize),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Window", "void SetTitle(const string& in)",
                                         asMETHOD(Window, SetTitle),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Window", "void Resize(const Vec2UI& in)",
                                         asMETHOD(Window, Resize),
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
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void SetClearColor(const Color& in)",
                                         asMETHOD(Renderer, SetClearColor),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void DrawLine(const Vec2& in, const Vec2& in, const Color& in, float, bool)",
                                         asMETHODPR(Renderer, DrawLine, (const Vec2&, const Vec2&, const Color&, float, bool), void),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void DrawRect(const Rect& in, const Color& in, float, bool)",
                                         asMETHODPR(Renderer, DrawRect, (const Rect&, const Color&, float, bool), void),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void DrawCircle(const Circle& in, const Color& in, uint32, float, bool)",
                                         asMETHODPR(Renderer, DrawCircle, (const Circle&, const Color&, uint32_t, float, bool), void),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void FillRect(const Rect& in, const Color& in, float, bool)",
                                         asMETHODPR(Renderer, FillRect, (const Rect&, const Color&, float, bool), void),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void DrawImage(const Image& in, const Region& in, const Region& in, const Color& in, Degrees, const Vec2& in, FlipFlags, float, bool)",
                                         asFUNCTION(+[](Renderer* r, const Image& img, const Region& src, const Region& dst, const Color& color, Degrees rot, const Vec2& center, Flags<Flip> flip, float z_order, bool use_camera) {
                                             r->DrawImage(img, src, dst, color, rot, center, flip, z_order, use_camera);
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void DrawImage9Grid(const Image& in, const Region& in, const Region& in, const Color& in, const Image9Grid& in, float, float, bool)",
                                         asMETHODPR(Renderer, DrawImage9Grid, (const Image&, const Region&, const Region&, const Color&, const Image9Grid&, float, float, bool), void),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void Clear()",
                                         asMETHOD(Renderer, Clear),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Renderer", "void Present()",
                                         asMETHOD(Renderer, Present),
                                         asCALL_THISCALL));
}

void bindTimer(asIScriptEngine* engine) {
    // TimerID enum
    AS_CALL(engine->RegisterEnum("TimerID"));
    
    // Time
    AS_CALL(engine->RegisterObjectMethod("Time", "void Update()",
                                         asMETHOD(Time, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Time", "TimeType GetCurrentTime() const",
                                         asMETHOD(Time, GetCurrentTime),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Time", "TimeType GetElapseTime() const",
                                         asMETHOD(Time, GetElapseTime),
                                         asCALL_THISCALL));
    
    // Timer
    AS_CALL(engine->RegisterObjectMethod("Timer", "void SetInterval(TimeType)",
                                         asMETHOD(Timer, SetInterval),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Update(TimeType)",
                                         asMETHOD(Timer, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Start()",
                                         asMETHOD(Timer, Start),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Stop()",
                                         asMETHOD(Timer, Stop),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Rewind()",
                                         asMETHOD(Timer, Rewind),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void SetLoop(int)",
                                         asMETHOD(Timer, SetLoop),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void Pause()",
                                         asMETHOD(Timer, Pause),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "TimeType GetInterval() const",
                                         asMETHOD(Timer, GetInterval),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "TimerEventType GetEventType() const",
                                         asMETHOD(Timer, GetEventType),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Timer", "void SetEventType(TimerEventType)",
                                         asMETHOD(Timer, SetEventType),
                                         asCALL_THISCALL));
    
    // TimerManager
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "Timer& Create(TimeType, TimerEventType, int)",
                                         asMETHOD(TimerManager, Create),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "void RemoveTimer(TimerID)",
                                         asMETHOD(TimerManager, RemoveTimer),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "void Clear()",
                                         asMETHOD(TimerManager, Clear),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "void Update(TimeType)",
                                         asMETHOD(TimerManager, Update),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("TimerManager", "Timer@ Find(TimerID)",
                                         asMETHOD(TimerManager, Find),
                                         asCALL_THISCALL));
}

void bindTrigger(asIScriptEngine* engine) {
    // TriggerEnterEvent
    AS_CALL(engine->RegisterObjectMethod("TriggerEnterEvent", "TriggerEventType GetType() const",
                                         asMETHOD(TriggerEnterEvent, GetType),
                                         asCALL_THISCALL));
    
    // TriggerLeaveEvent
    AS_CALL(engine->RegisterObjectMethod("TriggerLeaveEvent", "TriggerEventType GetType() const",
                                         asMETHOD(TriggerLeaveEvent, GetType),
                                         asCALL_THISCALL));
    
    // TriggerTouchEvent
    AS_CALL(engine->RegisterObjectMethod("TriggerTouchEvent", "TriggerEventType GetType() const",
                                         asMETHOD(TriggerTouchEvent, GetType),
                                         asCALL_THISCALL));
    
    // Trigger
    AS_CALL(engine->RegisterObjectMethod("Trigger", "const PhysicsActor@ GetActor() const",
                                         asMETHOD(Trigger, GetActor),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "void SetEventType(TriggerEventType)",
                                         asMETHOD(Trigger, SetEventType),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "TriggerEventType GetEventType() const",
                                         asMETHOD(Trigger, GetEventType),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "void EnableTriggerEveryFrameWhenTouch(bool)",
                                         asMETHOD(Trigger, EnableTriggerEveryFrameWhenTouch),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "bool IsTriggerEveryFrameWhenTouch() const",
                                         asMETHOD(Trigger, IsTriggerEveryFrameWhenTouch),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Trigger", "void Update()",
                                         asMETHOD(Trigger, Update),
                                         asCALL_THISCALL));
    
    // TriggerComponentManager
    AS_CALL(engine->RegisterObjectMethod("TriggerComponentManager", "void Update()",
                                         asMETHOD(TriggerComponentManager, Update),
                                         asCALL_THISCALL));
}

void bindContext(asIScriptEngine* engine) {
    // GameContext static methods
    AS_CALL(engine->RegisterGlobalFunction("void GameContext_Init()",
                                         asFUNCTION(GameContext::Init),
                                         asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("void GameContext_Destroy()",
                                         asFUNCTION(GameContext::Destroy),
                                         asCALL_CDECL));
    AS_CALL(engine->RegisterGlobalFunction("GameContext@ GameContext_GetInst()",
                                         asFUNCTION(GameContext::GetInst),
                                         asCALL_CDECL));
    
    // Methods inherited from CommonContext
    AS_CALL(engine->RegisterObjectMethod("GameContext", "Entity CreateEntity()",
                                         asMETHOD(GameContext, CreateEntity),
                                         asCALL_THISCALL));
    
    // Getter methods for public member variables (because they are unique_ptr)
    AS_CALL(engine->RegisterObjectMethod("GameContext", "Window@ get_m_window()",
                                         asFUNCTION(+[](GameContext* ctx) -> Window* { return ctx->m_window.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "Renderer@ get_m_renderer()",
                                         asFUNCTION(+[](GameContext* ctx) -> Renderer* { return ctx->m_renderer.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "Time@ get_m_time()",
                                         asFUNCTION(+[](GameContext* ctx) -> Time* { return ctx->m_time.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "PhysicsScene@ get_m_physics_scene()",
                                         asFUNCTION(+[](GameContext* ctx) -> PhysicsScene* { return ctx->m_physics_scene.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "AssetsManager@ get_m_assets_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> AssetsManager* { return ctx->m_assets_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "SpriteManager@ get_m_sprite_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> SpriteManager* { return ctx->m_sprite_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "RelationshipManager@ get_m_relationship_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> RelationshipManager* { return ctx->m_relationship_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "LevelManager@ get_m_level_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> LevelManager* { return ctx->m_level_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "TimerManager@ get_m_timer_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> TimerManager* { return ctx->m_timer_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "UIComponentManager@ get_m_ui_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> UIComponentManager* { return ctx->m_ui_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "TriggerComponentManager@ get_m_trigger_component_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> TriggerComponentManager* { return ctx->m_trigger_component_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("GameContext", "TilemapComponentManager@ get_m_tilemap_component_manager()",
                                         asFUNCTION(+[](GameContext* ctx) -> TilemapComponentManager* { return ctx->m_tilemap_component_manager.get(); }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectProperty("GameContext", "Camera m_camera",
                                          asOFFSET(CommonContext, m_camera)));
}

void bindAssetsManager(asIScriptEngine* engine) {
    // Specialized versions of GetManager() template method (according to conditions in asset_manager.hpp)
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "ImageManager@ GetImageManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> ImageManager* {
                                             return &mgr->GetManager<Image>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "TilemapManager@ GetTilemapManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> TilemapManager* {
                                             return &mgr->GetManager<Tilemap>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "AnimationManager@ GetAnimationManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> AnimationManager* {
                                             return &mgr->GetManager<Animation>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "LevelManager@ GetLevelManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> LevelManager* {
                                             return &mgr->GetManager<Level>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "PrefabManager@ GetPrefabManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> PrefabManager* {
                                             return &mgr->GetManager<Prefab>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "FontManager@ GetFontManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> FontManager* {
                                             return &mgr->GetManager<Font>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    AS_CALL(engine->RegisterObjectMethod("AssetsManager", "ScriptBinaryDataManager@ GetScriptBinaryDataManager()",
                                         asFUNCTION(+[](AssetsManager* mgr) -> ScriptBinaryDataManager* {
                                             return &mgr->GetManager<ScriptBinaryData>();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
}

void bindMath(asIScriptEngine* engine) {
    bindVec2(engine);
    bindDegrees(engine);
    bindRadians(engine);
    bindColor(engine);
    bindTransform(engine);
    bindOtherMath(engine);
}

void BindTLModule(asIScriptEngine* engine) {
    TL_RETURN_IF_NULL_WITH_LOG(engine, LOGE, "angel script engine is null!");

    AS_CALL(engine->SetDefaultNamespace("TL"));

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
    
    // Then register all methods, properties, etc. (actual binding)
    bindMath(engine);
    bindEntity(engine);
    bindLevel(engine);
    bindImage(engine);
    bindFont(engine);
    bindSprite(engine);
    bindTilemap(engine);
    bindUI(engine);
    bindPhysics(engine);
    bindRelationship(engine);
    bindWindow(engine);
    bindRenderer(engine);
    bindTimer(engine);
    bindTrigger(engine);
    bindContext(engine);
    bindAssetsManager(engine);
    
    // Bind Flags types
    bindFlagsType<HitType>(engine, "HitTypeFlags");
    bindFlagsType<UIAnchor>(engine, "UIAnchorFlags");
    bindFlagsType<Flip>(engine, "FlipFlags");
    
    // Register global function to access ImageManager
    // AS_CALL(engine->RegisterGlobalFunction("ImageManager@ GetImageManager()",
    //                                        asFUNCTION(GetImageManager),
    //                                        asCALL_CDECL));

    AS_CALL(engine->SetDefaultNamespace(""));
}
