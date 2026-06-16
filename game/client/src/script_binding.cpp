#include "client/script/script_client_binding.hpp"

#include "common/asset_manager.hpp"
#include "common/debug_drawer.hpp"
#include "common/entity.hpp"
#include "common/transform.hpp"

#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/event.hpp"
#include "common/net/net.hpp"
#include "common/physics.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/lua_event_listener.hpp"
#include "common/script/script.hpp"
#include "common/static_collision.hpp"
#include "common/tilemap.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/timer.hpp"
#include "common/trigger.hpp"

#include "client/animation_player.hpp"
#include "client/camera.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/input/input.hpp"
#include "client/input/mouse.hpp"
#include "client/renderer.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "client/ui.hpp"
#include "client/window.hpp"

static void registerLuaScriptEventBindings(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginClass<EventSystem>("EventSystem")
            TL_BIND_LUA_EVENT_LISTENER(UIMouseHoverEvent, "UIMouseHoverEvent")
                TL_BIND_LUA_EVENT_LISTENER(UIMouseDownEvent, "UIMouseDownEvent")
                    TL_BIND_LUA_EVENT_LISTENER(UIMouseUpEvent, "UIMouseUpEvent")
                        TL_BIND_LUA_EVENT_LISTENER(UIMouseClickedEvent,
                                                   "UIMouseClickedEvent")
                            TL_BIND_LUA_EVENT_LISTENER(UICheckToggledEvent,
                                                       "UICheckToggledEvent")
                                TL_BIND_LUA_EVENT_LISTENER(UIDragEvent,
                                                           "UIDragEvent")
        .endClass();
}

// clang-format off

static void bindInput(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
            .beginNamespace("ActionState")
                .addProperty("Pressed",
                             +[]() { return static_cast<int>(Action::State::Pressed); })
                .addProperty("Pressing",
                             +[]() { return static_cast<int>(Action::State::Pressing); })
                .addProperty("Released",
                             +[]() { return static_cast<int>(Action::State::Released); })
                .addProperty("Releasing",
                             +[]() { return static_cast<int>(Action::State::Releasing); })
            .endNamespace()
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

static void bindCamera(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
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
        .beginNamespace("TL_Client")
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

static void bindDrawOrder(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
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

static void bindAnimationPlayer(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
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

static void bindTilemapRenderComponent(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
            .beginClass<TilemapLayerRenderComponent>("TilemapRenderComponent")
                .addFunction("GetLayer", &TilemapLayerRenderComponent::GetLayer)
                .addFunction("GetTilemap", &TilemapLayerRenderComponent::GetTilemap)
            .endClass()
            .beginClass<TilemapLayerRenderComponentManager>(
                "TilemapRenderComponentManager")
                .addFunction("Get",
                             +[](TilemapLayerRenderComponentManager* m, Entity e) {
                                 return m->Get(e);
                             })
                .addFunction("Has",
                             +[](TilemapLayerRenderComponentManager* m, Entity e) {
                                 return m->Has(e);
                             })
            .endClass()
        .endNamespace();
}

static void bindUI(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
            .beginClass<UITextInput>("UITextInput")
                .addProperty("m_align", &UITextInput::m_align, true)
                .addProperty("m_color", &UITextInput::m_color, true)
                .addFunction("GetText", +[](const UITextInput* t) -> std::string {
                    return t->GetText().str();
                })
                .addFunction("SetText", +[](UITextInput* t, const std::string& text) {
                    t->SetText(text);
                })
                .addFunction("GetFontPt", &UITextInput::GetFontPt)
                .addFunction("SetFontPt", &UITextInput::SetFontPt)
                .addFunction("GetFontFilename", +[](const UITextInput* t) -> std::string {
                    auto font = t->GetFont();
                    if (auto filename = font.GetFilename()) {
                        return filename->string();
                    }
                    return "";
                })
                .addFunction("GetFont", &UITextInput::GetFont)
                .addFunction("SetFont", &UITextInput::SetFont)
                .addFunction("GetCursorPos", +[](const UITextInput* t) -> int {
                    return static_cast<int>(t->GetCursorPos());
                })
                .addFunction("SetCursorPos", +[](UITextInput* t, int pos) {
                    t->SetCursorPos(static_cast<size_t>(pos));
                })
                .addFunction("MoveCursorLeft", &UITextInput::MoveCursorLeft)
                .addFunction("MoveCursorRight", &UITextInput::MoveCursorRight)
                .addFunction("MoveCursorHome", &UITextInput::MoveCursorHome)
                .addFunction("MoveCursorEnd", &UITextInput::MoveCursorEnd)
                .addFunction("DeleteBeforeCursor", &UITextInput::DeleteBeforeCursor)
                .addFunction("DeleteAfterCursor", &UITextInput::DeleteAfterCursor)
                .addFunction("GetTextImageSize", &UITextInput::GetTextImageSize)
                .addFunction("GetCursorX", &UITextInput::GetCursorX)
                .addFunction("RefreshText", &UITextInput::RefreshText)
            .endClass()
            .beginClass<UIWidget>("UIWidget")
                .addProperty("m_use_clip", &UIWidget::m_use_clip, true)
                .addProperty("m_disabled", &UIWidget::m_disabled, true)
                .addProperty("m_selected", &UIWidget::m_selected, true)
                .addProperty("m_can_be_selected", &UIWidget::m_can_be_selected, true)
                .addProperty("m_margin", &UIWidget::m_margin, true)
                .addProperty("m_padding", &UIWidget::m_padding, true)
                .addFunction("GetTextInput", +[](UIWidget* w) -> UITextInput* {
                    return w->m_text_input.get();
                })
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

static void bindClientUIEvents(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
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
        .endNamespace();
}

static void bindClientContext(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Client")
            .deriveClass<ClientContext, CommonContext>("GameContext")
                .addFunction("GetCamera", +[](ClientContext* ctx) { return &ctx->m_camera; })
                .addFunction("GetSpriteManager",
                             +[](ClientContext* ctx) -> SpriteManager* {
                                 return ctx->m_sprite_manager.get();
                             })
                .addFunction("GetDrawOrderManager",
                             +[](ClientContext* ctx) -> DrawOrderManager* {
                                 return ctx->m_draw_order_manager.get();
                             })
                .addFunction("GetRenderer",
                             +[](ClientContext* ctx) -> Renderer* {
                                 return ctx->m_renderer.get();
                             })
                .addFunction("GetWindow",
                             +[](ClientContext* ctx) -> Window* {
                                 return ctx->m_window.get();
                             })
                .addFunction("GetInputManager",
                             +[](ClientContext* ctx) -> InputManager* {
                                 return ctx->m_input_manager.get();
                             })
                .addFunction("GetAnimationPlayerManager",
                             +[](ClientContext* ctx) -> AnimationPlayerManager* {
                                 return ctx->m_animation_player_manager.get();
                             })
                .addFunction("GetUIManager",
                             +[](ClientContext* ctx) -> UIComponentManager* {
                                 return ctx->m_ui_manager.get();
                             })
                .addFunction("GetTilemapRenderComponentManager",
                             +[](ClientContext* ctx)
                                 -> TilemapLayerRenderComponentManager* {
                                 return ctx->m_tilemap_layer_render_component_manager
                                     .get();
                             })
            .addFunction("ConnectToServer", &ClientContext::ConnectToServer)
            .addFunction("GetNetPeer", +[](ClientContext* ctx) -> UDPPeer& {
                            return ctx->m_net_peer;
                        })
            .addFunction("GetConfig",
                         +[](ClientContext* ctx) -> const ClientConfig* {
                             return &ctx->GetConfig();
                         })
            .endClass()
            .addFunction("GetContext", +[]() -> ClientContext* {
                return &ClientContext::GetInst();
            })
        .endNamespace();
}

// clang-format on

void BindClientModule(lua_State* L) {
    registerLuaScriptEventBindings(L);
    bindTilemapRenderComponent(L);
    bindInput(L);
    bindCamera(L);
    bindSprite(L);
    bindDrawOrder(L);
    bindAnimationPlayer(L);
    bindUI(L);
    bindClientUIEvents(L);
    bindClientContext(L);
}
