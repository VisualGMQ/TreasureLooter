--!strict
-- luau-lsp definition file for the `TL_Client` C++ binding namespace.
-- Generated equivalent of client/src/script_binding.cpp.
-- Types defined in common.d.luau / schema.d.luau are referenced as globals.

export type Renderer = {}
export type Window = {}

export type Sprite = {
	m_image: ImageHandle,
	m_region: Region,
	m_size: Vec2,
	m_anchor: Vec2,
	m_color: Color,
	m_flip: number,
}

export type Action = {
	IsPressed: (self: Action, id: number?) -> boolean,
	IsPressing: (self: Action, id: number?) -> boolean,
	IsReleased: (self: Action, id: number?) -> boolean,
	IsReleasing: (self: Action, id: number?) -> boolean,
	IsRelease: (self: Action, id: number?) -> boolean,
	IsPress: (self: Action, id: number?) -> boolean,
}
export type Axis = { Value: (self: Axis, id: number?) -> number }
export type Axises = { Value: (self: Axises, id: number?) -> Vec2 }
export type InputManager = {
	GetAxis: (self: InputManager, name: string) -> Axis,
	GetAction: (self: InputManager, name: string) -> Action,
	MakeAxises: (self: InputManager, x_name: string, y_name: string) -> Axises,
	AcceptFingerAxisEvent: (self: InputManager, event: any) -> (),
	AcceptFingerButton: (self: InputManager, name: string, state: number) -> (),
}

export type MouseButton = {
	IsPressing: (self: MouseButton) -> boolean,
	IsReleasing: (self: MouseButton) -> boolean,
	IsReleased: (self: MouseButton) -> boolean,
	IsPressed: (self: MouseButton) -> boolean,
	IsPress: (self: MouseButton) -> boolean,
	IsRelease: (self: MouseButton) -> boolean,
	GetLastDownTime: (self: MouseButton) -> TimeType,
	GetLastUpTime: (self: MouseButton) -> TimeType,
}

export type Camera = {
	GetPosition: (self: Camera) -> Vec2,
	GetScale: (self: Camera) -> number,
	MoveTo: (self: Camera, pos: Vec2) -> (),
	Move: (self: Camera, delta: Vec2) -> (),
	ChangeScale: (self: Camera, scale: number) -> (),
}

export type SpriteManager = {
	Get: (self: SpriteManager, entity: Entity) -> Sprite?,
	Has: (self: SpriteManager, entity: Entity) -> boolean,
	IsEnable: (self: SpriteManager, entity: Entity) -> boolean,
	Enable: (self: SpriteManager, entity: Entity) -> (),
	Disable: (self: SpriteManager, entity: Entity) -> (),
	RegisterEntity: (self: SpriteManager, entity: Entity, def: Sprite) -> (),
}

export type DrawOrder = {
	m_z_order: number,
	m_enable_y_sorting: boolean,
	GetGlobalOrder: (self: DrawOrder) -> number,
}
export type DrawOrderManager = {
	Get: (self: DrawOrderManager, entity: Entity) -> DrawOrder?,
	Has: (self: DrawOrderManager, entity: Entity) -> boolean,
	RegisterEntity: (self: DrawOrderManager, entity: Entity, def: DrawOrderDefinition) -> (),
}

export type AnimationPlayer = {
	Play: (self: AnimationPlayer) -> (),
	Pause: (self: AnimationPlayer) -> (),
	Stop: (self: AnimationPlayer) -> (),
	Rewind: (self: AnimationPlayer) -> (),
	SetLoop: (self: AnimationPlayer, loop: number) -> (),
	IsPlaying: (self: AnimationPlayer) -> boolean,
	GetLoopCount: (self: AnimationPlayer) -> number,
	GetCurTime: (self: AnimationPlayer) -> number,
	GetMaxTime: (self: AnimationPlayer) -> number,
	ChangeAnimation: (self: AnimationPlayer, handle: AnimationHandle) -> (),
	ClearAnimation: (self: AnimationPlayer) -> (),
	HasAnimation: (self: AnimationPlayer) -> boolean,
	Sync: (self: AnimationPlayer, entity: Entity) -> (),
	SetRate: (self: AnimationPlayer, rate: number) -> (),
	GetRate: (self: AnimationPlayer) -> number,
	EnableAutoPlay: (self: AnimationPlayer, enable: boolean) -> (),
	IsAutoPlayEnabled: (self: AnimationPlayer) -> boolean,
}

export type MultiAnimationPlayer = {
	GetAnimation: (self: MultiAnimationPlayer, index: number) -> AnimationPlayer?,
	GetAnimationCount: (self: MultiAnimationPlayer) -> number,
	PlayAll: (self: MultiAnimationPlayer) -> (),
	PauseAll: (self: MultiAnimationPlayer) -> (),
	StopAll: (self: MultiAnimationPlayer) -> (),
	RewindAll: (self: MultiAnimationPlayer) -> (),
}

export type MultiAnimationPlayerManager = {
	Get: (self: MultiAnimationPlayerManager, entity: Entity) -> MultiAnimationPlayer?,
	Has: (self: MultiAnimationPlayerManager, entity: Entity) -> boolean,
	IsEnable: (self: MultiAnimationPlayerManager, entity: Entity) -> boolean,
	Enable: (self: MultiAnimationPlayerManager, entity: Entity) -> (),
	Disable: (self: MultiAnimationPlayerManager, entity: Entity) -> (),
	RegisterEntity: (self: MultiAnimationPlayerManager, entity: Entity, def: MultiAnimationPlayerDefinition) -> (),
}

export type TilemapRenderComponent = {
	GetLayer: (self: TilemapRenderComponent) -> TilemapLayer?,
	GetTilemap: (self: TilemapRenderComponent) -> TilemapHandle,
}
export type TilemapRenderComponentManager = {
	Get: (self: TilemapRenderComponentManager, entity: Entity) -> TilemapRenderComponent?,
	Has: (self: TilemapRenderComponentManager, entity: Entity) -> boolean,
}

export type UITextInput = {
	m_align: number,
	m_color: Color,
	GetText: (self: UITextInput) -> string,
	SetText: (self: UITextInput, text: string) -> (),
	GetFontPt: (self: UITextInput) -> number,
	SetFontPt: (self: UITextInput, pt: number) -> (),
	GetFontFilename: (self: UITextInput) -> string,
	GetFont: (self: UITextInput) -> FontHandle,
	SetFont: (self: UITextInput, font: FontHandle) -> (),
	GetCursorPos: (self: UITextInput) -> number,
	SetCursorPos: (self: UITextInput, pos: number) -> (),
	MoveCursorLeft: (self: UITextInput) -> (),
	MoveCursorRight: (self: UITextInput) -> (),
	MoveCursorHome: (self: UITextInput) -> (),
	MoveCursorEnd: (self: UITextInput) -> (),
	DeleteBeforeCursor: (self: UITextInput) -> (),
	DeleteAfterCursor: (self: UITextInput) -> (),
	GetTextImageSize: (self: UITextInput) -> Vec2,
	GetCursorX: (self: UITextInput) -> number,
	RefreshText: (self: UITextInput) -> (),
}
export type UIWidget = {
	m_use_clip: boolean,
	m_disabled: boolean,
	m_selected: boolean,
	m_can_be_selected: boolean,
	m_margin: Vec2,
	m_padding: Vec2,
	GetTextInput: (self: UIWidget) -> UITextInput?,
}
export type UIComponentManager = {
	Get: (self: UIComponentManager, entity: Entity) -> UIWidget?,
	Has: (self: UIComponentManager, entity: Entity) -> boolean,
}

export type UIMouseHoverEvent = { m_entity: Entity }
export type UIMouseDownEvent = {
	m_entity: Entity,
	GetButton: (self: UIMouseDownEvent) -> MouseButton,
}
export type UIMouseUpEvent = {
	m_entity: Entity,
	GetButton: (self: UIMouseUpEvent) -> MouseButton,
}
export type UIMouseClickedEvent = { m_entity: Entity }
export type UICheckToggledEvent = { m_entity: Entity, m_checked: boolean }
export type UIDragEvent = { m_entity: Entity }

export type ClientContext = CommonContext & {
	GetCamera: (self: ClientContext) -> Camera,
	GetSpriteManager: (self: ClientContext) -> SpriteManager,
	GetDrawOrderManager: (self: ClientContext) -> DrawOrderManager,
	GetRenderer: (self: ClientContext) -> Renderer,
	GetWindow: (self: ClientContext) -> Window,
	GetInputManager: (self: ClientContext) -> InputManager,
	GetMultiAnimationPlayerManager: (self: ClientContext) -> MultiAnimationPlayerManager,
	GetUIManager: (self: ClientContext) -> UIComponentManager,
	GetTilemapRenderComponentManager: (self: ClientContext) -> TilemapRenderComponentManager,
	GetNetPeer: (self: ClientContext) -> UDPPeer,
	GetConfig: (self: ClientContext) -> ClientConfig,
	ConnectToServer: (self: ClientContext, address: NetAddress) -> (),
}

export type ActionStateNamespace = {
	Pressed: number,
	Pressing: number,
	Released: number,
	Releasing: number,
}

-- The runtime global table created by `beginNamespace("TL_Client")`.
declare TL_Client: {
	-- constructors
	Sprite: () -> Sprite,
	-- schema definition constructors registered under this namespace
	DrawOrderDefinition: () -> DrawOrderDefinition,
	AnimationPlayerDefinition: () -> AnimationPlayerDefinition,
	MultiAnimationPlayerDefinition: () -> MultiAnimationPlayerDefinition,

	-- sub-namespaces
	ActionState: ActionStateNamespace,

	-- free functions
	GetContext: () -> ClientContext,
}
