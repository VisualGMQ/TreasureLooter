---@meta
-- LuaLS (LuaCATS) definition file for the `TL_Client` C++ binding namespace.
-- Hand-written equivalent of client/src/script_binding.cpp.
-- Types declared in common_meta.lua / schema_meta.lua are referenced as globals.

---@class Renderer

---@class Window

-- `Sprite` is a C++ alias of `SpriteDefinition` (client/include/client/sprite.hpp:
-- `using Sprite = SpriteDefinition;`). It is bound once by schema_meta.lua as
-- `SpriteDefinition`, so here it is only a LuaCATS alias, not a second runtime
-- class (`TL_Client.Sprite` does not exist at runtime).
---@alias Sprite SpriteDefinition

---@class Action
---@field IsPressed fun(self: Action, id: number?): boolean
---@field IsPressing fun(self: Action, id: number?): boolean
---@field IsReleased fun(self: Action, id: number?): boolean
---@field IsReleasing fun(self: Action, id: number?): boolean
---@field IsRelease fun(self: Action, id: number?): boolean
---@field IsPress fun(self: Action, id: number?): boolean

---@class Axis
---@field Value fun(self: Axis, id: number?): number

---@class Axises
---@field Value fun(self: Axises, id: number?): Vec2

---@class InputManager
---@field GetAxis fun(self: InputManager, name: string): Axis
---@field GetAction fun(self: InputManager, name: string): Action
---@field MakeAxises fun(self: InputManager, x_name: string, y_name: string): Axises
---@field AcceptFingerAxisEvent fun(self: InputManager, event: any)
---@field AcceptFingerButton fun(self: InputManager, name: string, state: number)

---@class MouseButton
---@field IsPressing fun(self: MouseButton): boolean
---@field IsReleasing fun(self: MouseButton): boolean
---@field IsReleased fun(self: MouseButton): boolean
---@field IsPressed fun(self: MouseButton): boolean
---@field IsPress fun(self: MouseButton): boolean
---@field IsRelease fun(self: MouseButton): boolean
---@field GetLastDownTime fun(self: MouseButton): TimeType
---@field GetLastUpTime fun(self: MouseButton): TimeType

---@class Camera
---@field GetPosition fun(self: Camera): Vec2
---@field GetScale fun(self: Camera): number
---@field MoveTo fun(self: Camera, pos: Vec2)
---@field Move fun(self: Camera, delta: Vec2)
---@field ChangeScale fun(self: Camera, scale: number)
---@field SetBoundary fun(self: Camera, boundary: Rect)

---@class SpriteManager
---@field Get fun(self: SpriteManager, entity: LogicEntity): Sprite?
---@field Has fun(self: SpriteManager, entity: LogicEntity): boolean
---@field IsEnable fun(self: SpriteManager, entity: LogicEntity): boolean
---@field Enable fun(self: SpriteManager, entity: LogicEntity)
---@field Disable fun(self: SpriteManager, entity: LogicEntity)
---@field RegisterEntity fun(self: SpriteManager, entity: LogicEntity, def: Sprite)

---@class DrawOrder
---@field m_z_order number
---@field m_enable_y_sorting boolean
---@field GetGlobalOrder fun(self: DrawOrder): number

---@class DrawOrderManager
---@field Get fun(self: DrawOrderManager, entity: LogicEntity): DrawOrder?
---@field Has fun(self: DrawOrderManager, entity: LogicEntity): boolean
---@field RegisterEntity fun(self: DrawOrderManager, entity: LogicEntity, def: DrawOrderDefinition)

---@class AnimationPlayer
---@field Play fun(self: AnimationPlayer)
---@field Pause fun(self: AnimationPlayer)
---@field Stop fun(self: AnimationPlayer)
---@field Rewind fun(self: AnimationPlayer)
---@field SetLoop fun(self: AnimationPlayer, loop: number)
---@field SetCurTime fun(self: AnimationPlayer, time: TimeType)
---@field IsPlaying fun(self: AnimationPlayer): boolean
---@field GetLoopCount fun(self: AnimationPlayer): number
---@field GetCurTime fun(self: AnimationPlayer): number
---@field GetMaxTime fun(self: AnimationPlayer): number
---@field ChangeAnimation fun(self: AnimationPlayer, handle: AnimationHandle)
---@field ClearAnimation fun(self: AnimationPlayer)
---@field HasAnimation fun(self: AnimationPlayer): boolean
---@field Sync fun(self: AnimationPlayer, entity: LogicEntity)
---@field SetRate fun(self: AnimationPlayer, rate: number)
---@field GetRate fun(self: AnimationPlayer): number
---@field EnableAutoPlay fun(self: AnimationPlayer, enable: boolean)
---@field IsAutoPlayEnabled fun(self: AnimationPlayer): boolean
---@field GetID fun(self: AnimationPlayer): number

---@class AnimationPlayerManager
---@field AddComponent fun(self: AnimationPlayerManager, entity: LogicEntity, def: AnimationPlayerDefinition): AnimationPlayer?
---@field Get fun(self: AnimationPlayerManager, entity: LogicEntity, index: number): AnimationPlayer?
---@field GetComponentSize fun(self: AnimationPlayerManager, entity: LogicEntity): number
---@field RemoveComponent fun(self: AnimationPlayerManager, entity: LogicEntity, player: AnimationPlayer)
---@field Has fun(self: AnimationPlayerManager, entity: LogicEntity): boolean
---@field IsEnable fun(self: AnimationPlayerManager, entity: LogicEntity, player: AnimationPlayer): boolean
---@field Enable fun(self: AnimationPlayerManager, entity: LogicEntity, index: number)
---@field EnablePlayer fun(self: AnimationPlayerManager, entity: LogicEntity, player: AnimationPlayer)
---@field EnableAll fun(self: AnimationPlayerManager, entity: LogicEntity)
---@field Disable fun(self: AnimationPlayerManager, entity: LogicEntity, index: number)
---@field DisablePlayer fun(self: AnimationPlayerManager, entity: LogicEntity, player: AnimationPlayer)
---@field DisableAll fun(self: AnimationPlayerManager, entity: LogicEntity)
---@field RegisterEntity fun(self: AnimationPlayerManager, entity: LogicEntity, def: MultiAnimationPlayerDefinition)

---@class TilemapRenderComponent
---@field GetLayer fun(self: TilemapRenderComponent): TilemapLayer?
---@field GetTilemap fun(self: TilemapRenderComponent): TilemapHandle

---@class TilemapRenderComponentManager
---@field Get fun(self: TilemapRenderComponentManager, entity: LogicEntity): TilemapRenderComponent?
---@field Has fun(self: TilemapRenderComponentManager, entity: LogicEntity): boolean

---@class UITextInput
---@field m_align number
---@field m_color Color
---@field GetText fun(self: UITextInput): string
---@field SetText fun(self: UITextInput, text: string)
---@field GetFontPt fun(self: UITextInput): number
---@field SetFontPt fun(self: UITextInput, pt: number)
---@field GetFontFilename fun(self: UITextInput): string
---@field GetFont fun(self: UITextInput): FontHandle
---@field SetFont fun(self: UITextInput, font: FontHandle)
---@field GetCursorPos fun(self: UITextInput): number
---@field SetCursorPos fun(self: UITextInput, pos: number)
---@field MoveCursorLeft fun(self: UITextInput)
---@field MoveCursorRight fun(self: UITextInput)
---@field MoveCursorHome fun(self: UITextInput)
---@field MoveCursorEnd fun(self: UITextInput)
---@field DeleteBeforeCursor fun(self: UITextInput)
---@field DeleteAfterCursor fun(self: UITextInput)
---@field GetTextImageSize fun(self: UITextInput): Vec2
---@field GetCursorX fun(self: UITextInput): number
---@field RefreshText fun(self: UITextInput)

---@class UIWidget
---@field m_use_clip boolean
---@field m_disabled boolean
---@field m_selected boolean
---@field m_can_be_selected boolean
---@field m_margin Vec2
---@field m_padding Vec2
---@field GetTextInput fun(self: UIWidget): UITextInput?

---@class UIComponentManager
---@field Get fun(self: UIComponentManager, entity: LogicEntity): UIWidget?
---@field Has fun(self: UIComponentManager, entity: LogicEntity): boolean

---@class UIMouseHoverEvent
---@field m_entity LogicEntity

---@class UIMouseDownEvent
---@field m_entity LogicEntity
---@field GetButton fun(self: UIMouseDownEvent): MouseButton

---@class UIMouseUpEvent
---@field m_entity LogicEntity
---@field GetButton fun(self: UIMouseUpEvent): MouseButton

---@class UIMouseClickedEvent
---@field m_entity LogicEntity

---@class UICheckToggledEvent
---@field m_entity LogicEntity
---@field m_checked boolean

---@class UIDragEvent
---@field m_entity LogicEntity

---@class AnimationEndEvent
---@field GetAnimationPlayerID fun(self: AnimationEndEvent): number
---@field GetEntity fun(self: AnimationEndEvent): LogicEntity
---@field GetAnimation fun(self: AnimationEndEvent): AnimationHandle

---@class ClientContext : CommonContext
---@field GetCamera fun(self: ClientContext): Camera
---@field GetPresentEntity fun(self: ClientContext, entity: LogicEntity): PresentEntity
---@field GetPresentTransformManager fun(self: ClientContext): PresentTransformManager
---@field GetSpriteManager fun(self: ClientContext): SpriteManager
---@field GetDrawOrderManager fun(self: ClientContext): DrawOrderManager
---@field GetRenderer fun(self: ClientContext): Renderer
---@field GetWindow fun(self: ClientContext): Window
---@field GetInputManager fun(self: ClientContext): InputManager
---@field GetAnimationPlayerManager fun(self: ClientContext): AnimationPlayerManager
---@field GetUIManager fun(self: ClientContext): UIComponentManager
---@field GetTilemapRenderComponentManager fun(self: ClientContext): TilemapRenderComponentManager
---@field GetNetPeer fun(self: ClientContext): UDPPeer
---@field GetConfig fun(self: ClientContext): ClientConfig
---@field GetDebugPanel fun(self: ClientContext): DebugPanel
---@field ConnectToServer fun(self: ClientContext, address: NetAddress)
---@field WindowCoordToWorld fun(self: ClientContext, window_pos: Vec2): Vec2
---@field WorldCoordToWindow fun(self: ClientContext, world_pos: Vec2): Vec2

---@class ActionStateNamespace
---@field Pressed number
---@field Pressing number
---@field Released number
---@field Releasing number

---@class DebugPanel
---@field RegisterCmd fun(self: DebugPanel, name: string, cmd: fun(args: string[]), hint_list: any[])
---@field ExecuteCmd fun(self: DebugPanel, name: string, args: string[])

-- The runtime global table created by `beginNamespace("TL_Client")`.
---@class TL_Client
-- constructors
-- schema definition constructors registered under this namespace
---@field DrawOrderDefinition fun(): DrawOrderDefinition
---@field AnimationPlayerDefinition fun(): AnimationPlayerDefinition
---@field MultiAnimationPlayerDefinition fun(): MultiAnimationPlayerDefinition
-- sub-namespaces
---@field ActionState ActionStateNamespace
-- free functions
---@field GetContext fun(): ClientContext
TL_Client = {}
