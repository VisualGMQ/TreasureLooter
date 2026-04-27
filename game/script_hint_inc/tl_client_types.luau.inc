-- Types bound from client/src/script_binding.cpp (TL.Client)

export type Sprite = {
	m_image: ImageHandle,
	m_region: Region,
	m_size: Vec2,
	m_anchor: Vec2,
	m_z_order: number,
	m_color: Color,
	m_flip: number,
} & () -> Sprite

export type Renderer = {}
export type Window = {}
export type UIWidget = {
	m_use_clip: boolean,
	m_disabled: boolean,
	m_selected: boolean,
	m_can_be_selected: boolean,
	m_margin: Vec2,
	m_padding: Vec2,
}
export type TilemapRenderComponent = {
	GetLayer: (self: TilemapRenderComponent) -> unknown,
	GetTilemap: (self: TilemapRenderComponent) -> TilemapHandle,
	GetTilemapCollision: (self: TilemapRenderComponent) -> TilemapCollisionInfo?,
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
	AcceptFingerAxisEvent: (self: InputManager, event: unknown) -> (),
	AcceptFingerButton: (self: InputManager, name: string, state: number) -> (),
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
	SetLoop: (self: AnimationPlayer, loop: boolean) -> (),
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
export type AnimationPlayerManager = {
	Get: (self: AnimationPlayerManager, entity: Entity) -> AnimationPlayer?,
	Has: (self: AnimationPlayerManager, entity: Entity) -> boolean,
	IsEnable: (self: AnimationPlayerManager, entity: Entity) -> boolean,
	Enable: (self: AnimationPlayerManager, entity: Entity) -> (),
	Disable: (self: AnimationPlayerManager, entity: Entity) -> (),
}

export type UIComponentManager = { Get: (self: UIComponentManager, entity: Entity) -> UIWidget?, Has: (self: UIComponentManager, entity: Entity) -> boolean }
export type TilemapRenderComponentManager = { Get: (self: TilemapRenderComponentManager, entity: Entity) -> TilemapRenderComponent?, Has: (self: TilemapRenderComponentManager, entity: Entity) -> boolean }

export type ClientContext = {
	GetCamera: (self: ClientContext) -> Camera,
	GetScriptManager: (self: ClientContext) -> ScriptComponentManager,
	GetAssetsManager: (self: ClientContext) -> AssetsManager,
	GetSceneManager: (self: ClientContext) -> SceneManager,
	GetTime: (self: ClientContext) -> Time,
	GetTimerManager: (self: ClientContext) -> TimerManager,
	GetTransformManager: (self: ClientContext) -> TransformManager,
	GetSpriteManager: (self: ClientContext) -> SpriteManager,
	GetDrawOrderManager: (self: ClientContext) -> DrawOrderManager,
	GetRenderer: (self: ClientContext) -> Renderer,
	GetWindow: (self: ClientContext) -> Window,
	GetInputManager: (self: ClientContext) -> InputManager,
	GetTriggerComponentManager: (self: ClientContext) -> TriggerComponentManager,
	GetRelationshipManager: (self: ClientContext) -> RelationshipManager,
	GetAnimationPlayerManager: (self: ClientContext) -> AnimationPlayerManager,
	GetBindPointsComponentManager: (self: ClientContext) -> BindPointsComponentManager,
	GetUIManager: (self: ClientContext) -> UIComponentManager,
	GetTilemapRenderComponentManager: (self: ClientContext) -> TilemapRenderComponentManager,
	GetTilemapCollisionComponentManager: (self: ClientContext) -> TilemapCollisionComponentManager,
	GetCCTManager: (self: ClientContext) -> CCTManager,
	GetPhysicsScene: (self: ClientContext) -> PhysicsScene,
	GetStaticCollisionManager: (self: ClientContext) -> StaticCollisionManager,
	GetEventDebugger: (self: ClientContext) -> EventDebugger,
	GetDebugDraw: (self: ClientContext) -> DebugDraw,
}

export type GetClientContext = () -> ClientContext
