--!strict
-- luau-lsp definition file for the `TL_Common` C++ binding namespace.
-- Generated equivalent of common/src/scripts/script_binding.cpp.
-- Loaded through luau-lsp's `types.definitionFiles`, so every type below is a
-- global type and `TL_Common` is a global value (no `require` needed).

export type Entity = number
export type TimeType = number
export type TimerID = number
export type EventListenerID = number

-- Opaque flags wrapper produced by `TL_Common.UDPPacketFlags(...)`.
export type UDPPacketFlags = number

export type UUID = {
	IsValid: (self: UUID) -> boolean,
}

declare class Vec2
	x: number
	y: number
	function Length(self): number
	function LengthSquared(self): number
	function Normalize(self): Vec2
	function Dot(self, other: Vec2): number
	function Cross(self, other: Vec2): number
	function __add(self, other: Vec2): Vec2
	function __sub(self, other: Vec2): Vec2
	function __mul(self, other: Vec2 | number): Vec2
	function __div(self, other: Vec2 | number): Vec2
end

declare class Vec2UI
	x: number
	y: number
	function Length(self): number
	function LengthSquared(self): number
	function Normalize(self): Vec2UI
	function Dot(self, other: Vec2UI): number
	function Cross(self, other: Vec2UI): number
	function __add(self, other: Vec2UI): Vec2UI
	function __sub(self, other: Vec2UI): Vec2UI
	function __mul(self, other: Vec2UI | number): Vec2UI
	function __div(self, other: Vec2UI | number): Vec2UI
end

export type Color = {
	r: number,
	g: number,
	b: number,
	a: number,
}

export type DecompositionResult = {
	m_tangent: Vec2,
	m_normal: Vec2,
}

export type Degrees = {
	Value: (self: Degrees) -> number,
}

export type Radians = {
	Value: (self: Radians) -> number,
}

export type Transform = {
	m_position: Vec2,
	m_rotation: Degrees,
	m_size: Vec2,
	m_scale: Vec2,
	GetGlobalPosition: (self: Transform) -> Vec2,
}

export type Region = {
	m_topleft: Vec2,
	m_size: Vec2,
}

export type Path = {
	parent_path: (self: Path) -> Path,
	filename: (self: Path) -> string,
	extension: (self: Path) -> string,
	has_extension: (self: Path, ext: string?) -> boolean,
	is_absolute: (self: Path) -> boolean,
	is_relative: (self: Path) -> boolean,
	empty: (self: Path) -> boolean,
}

export type CollisionGroup = {
	Add: (self: CollisionGroup, group: CollisionGroupType) -> (),
	Remove: (self: CollisionGroup, group: CollisionGroupType) -> (),
	Has: (self: CollisionGroup, group: CollisionGroupType) -> boolean,
	Clear: (self: CollisionGroup) -> (),
	CanCollision: (self: CollisionGroup, o: CollisionGroup) -> boolean,
	GetUnderlying: (self: CollisionGroup) -> number,
	SetUnderlying: (self: CollisionGroup, underlying: number) -> (),
}

export type Image = {
	GetSize: (self: Image) -> Vec2,
	ChangeColorMask: (self: Image, color: Color) -> (),
}

export type ScriptBinaryData = {}

export type Animation = {}

export type Font = {
	IsValid: (self: Font) -> boolean,
	GetHeight: (self: Font) -> number,
	SetFontSize: (self: Font, size: number) -> (),
}

export type ImageHandle = { IsValid: (self: ImageHandle) -> boolean, GetFilename: (self: ImageHandle) -> Path?, GetUUID: (self: ImageHandle) -> UUID } & Image
export type FontHandle = { IsValid: (self: FontHandle) -> boolean, GetFilename: (self: FontHandle) -> Path?, GetUUID: (self: FontHandle) -> UUID } & Font
export type ScriptBinaryDataHandle = { IsValid: (self: ScriptBinaryDataHandle) -> boolean, GetFilename: (self: ScriptBinaryDataHandle) -> Path?, GetUUID: (self: ScriptBinaryDataHandle) -> UUID } & ScriptBinaryData
export type AnimationHandle = { IsValid: (self: AnimationHandle) -> boolean, GetFilename: (self: AnimationHandle) -> Path?, GetUUID: (self: AnimationHandle) -> UUID } & Animation
export type TilemapHandle = { IsValid: (self: TilemapHandle) -> boolean, GetFilename: (self: TilemapHandle) -> Path?, GetUUID: (self: TilemapHandle) -> UUID } & Tilemap
export type SceneHandle = { IsValid: (self: SceneHandle) -> boolean, GetFilename: (self: SceneHandle) -> Path?, GetUUID: (self: SceneHandle) -> UUID } & Scene

export type Scene = {
	Instantiate: (self: Scene, prefab: PrefabHandle, transform: Transform?) -> Entity,
	RemoveEntity: (self: Scene, entity: Entity) -> (),
	GetRootEntity: (self: Scene) -> Entity,
	GetUIRootEntity: (self: Scene) -> Entity,
}

export type BindPoint = {
	m_position: Vec2,
	m_name: string,
	GetGlobalPosition: (self: BindPoint) -> Vec2,
}
export type BindPoints = {
	m_bind_points: { BindPoint },
}
export type BindPointsComponentManager = {
	Get: (self: BindPointsComponentManager, entity: Entity) -> BindPoints,
	Has: (self: BindPointsComponentManager, entity: Entity) -> boolean,
	ToggleDebugDraw: (self: BindPointsComponentManager) -> (),
}

export type Tile = {
	m_image: ImageHandle,
	m_region: Region,
	m_id: number,
	m_tile_size: Vec2,
}

export type TilemapLayerTile = {
	m_gid: number,
	GetFlipValue: (self: TilemapLayerTile) -> number,
}

export type Tileset = {
	GetTile: (self: Tileset, gid: number) -> Tile?,
	HasTile: (self: Tileset, gid: number) -> boolean,
	GetTileSize: (self: Tileset) -> Vec2,
}

export type TilemapLayer = {
	GetType: (self: TilemapLayer) -> number,
	AsTiledLayer: (self: TilemapLayer) -> TilemapTileLayer?,
	AsImageLayer: (self: TilemapLayer) -> TilemapImageLayer?,
	AsObjectLayer: (self: TilemapLayer) -> TilemapObjectLayer?,
	GetName: (self: TilemapLayer) -> string,
}

export type TilemapTileLayer = TilemapLayer & {
	GetTile: (self: TilemapTileLayer, x: number, y: number) -> TilemapLayerTile?,
	GetSize: (self: TilemapTileLayer) -> Vec2,
}

export type TilemapObjectLayer = TilemapLayer & {
	GetObjectCount: (self: TilemapObjectLayer) -> number,
	GetObject: (self: TilemapObjectLayer, index: number) -> TilemapObject?,
}

export type TilemapImageLayer = TilemapLayer & {
	GetImage: (self: TilemapImageLayer) -> ImageHandle?,
	GetPosition: (self: TilemapImageLayer) -> Vec2,
}

export type TilemapObject = {
	GetType: (self: TilemapObject) -> number,
	GetName: (self: TilemapObject) -> string,
	IsVisiable: (self: TilemapObject) -> boolean,
	AsCircle: (self: TilemapObject) -> Circle?,
	AsRect: (self: TilemapObject) -> Rect?,
	AsPolygon: (self: TilemapObject) -> Polygon?,
	AsPoint: (self: TilemapObject) -> Vec2?,
}

export type TilemapCollisionInfo = { m_topleft: Vec2 }

export type Tilemap = {
	GetLayerCount: (self: Tilemap) -> number,
	GetLayer: (self: Tilemap, index: number) -> TilemapLayer?,
	GetTilesetCount: (self: Tilemap) -> number,
	GetTileset: (self: Tilemap, index: number) -> Tileset?,
	GetTile: (self: Tilemap, gid: number) -> Tile?,
	GetTileSize: (self: Tilemap) -> Vec2,
	GetFilename: (self: Tilemap) -> string,
}

export type Relationship = {
	AddChild: (self: Relationship, entity: Entity) -> (),
	Get: (self: Relationship, index: number) -> Entity,
	GetChildrenCount: (self: Relationship) -> number,
	GetParent: (self: Relationship) -> Entity,
	RemoveChild: (self: Relationship, entity: Entity) -> (),
	RemoveFromParent: (self: Relationship) -> (),
}

export type ScriptBinaryDataManager = {
	Load: (self: ScriptBinaryDataManager, path: string, force: boolean?) -> ScriptBinaryDataHandle,
	Find: (self: ScriptBinaryDataManager, path: string) -> ScriptBinaryDataHandle,
}
export type ScriptComponentManager = {
	Get: (self: ScriptComponentManager, entity: Entity) -> any,
	Has: (self: ScriptComponentManager, entity: Entity) -> boolean,
	IsEnable: (self: ScriptComponentManager, entity: Entity) -> boolean,
	SubscribeEvent: (self: ScriptComponentManager, entity: Entity, eventName: string) -> (),
}
export type TransformManager = {
	Get: (self: TransformManager, entity: Entity) -> Transform?,
	Has: (self: TransformManager, entity: Entity) -> boolean,
}
export type ImageManager = {
	Load: (self: ImageManager, path: string, force: boolean?) -> ImageHandle,
	Find: (self: ImageManager, path: string) -> ImageHandle,
}
export type TilemapManager = {
	Load: (self: TilemapManager, path: string, force: boolean?) -> TilemapHandle,
	Find: (self: TilemapManager, path: string) -> TilemapHandle,
}
export type AnimationManager = {
	Load: (self: AnimationManager, path: string, force: boolean?) -> AnimationHandle,
	Find: (self: AnimationManager, path: string) -> AnimationHandle,
}
export type SceneManager = {
	Load: (self: SceneManager, path: string, force: boolean?) -> SceneHandle,
	Find: (self: SceneManager, path: string) -> SceneHandle,
	Create: (self: SceneManager, definition: SceneDefinitionHandle) -> SceneHandle,
	Unload: (self: SceneManager, handle: SceneHandle) -> (),
	GetCurrentScene: (self: SceneManager) -> Scene?,
	Switch: (self: SceneManager, scene: SceneHandle) -> (),
}
export type FontManager = {
	Load: (self: FontManager, path: string, force: boolean?) -> FontHandle,
	Find: (self: FontManager, path: string) -> FontHandle,
}
export type AssetsManager = {
	GetScriptBinaryDataManager: (self: AssetsManager) -> ScriptBinaryDataManager,
	GetImageManager: (self: AssetsManager) -> ImageManager,
	GetTilemapManager: (self: AssetsManager) -> TilemapManager,
	GetAnimationManager: (self: AssetsManager) -> AnimationManager,
	GetSceneDefinitionManager: (self: AssetsManager) -> SceneDefinitionAssetManager,
	GetFontManager: (self: AssetsManager) -> FontManager,

	-- registered by the schema binding (see schema.d.luau)
	GetPrefabManager: (self: AssetsManager) -> PrefabAssetManager,
	GetCharacterDefinitionManager: (self: AssetsManager) -> CharacterDefinitionAssetManager,
	GetLevelDefinitionManager: (self: AssetsManager) -> LevelDefinitionAssetManager,
	GetInputConfigManager: (self: AssetsManager) -> InputConfigAssetManager,
	GetPhysicsShapeDefinitionManager: (self: AssetsManager) -> PhysicsShapeDefinitionAssetManager,
	GetUIWidgetDefinitionManager: (self: AssetsManager) -> UIWidgetDefinitionAssetManager,
	GetItemDefinitionManager: (self: AssetsManager) -> ItemDefinitionAssetManager,
	GetObjectDefinitionTableManager: (self: AssetsManager) -> ObjectDefinitionTableAssetManager,
}

export type TilemapCollisionComponent = {
	GetLayer: (self: TilemapCollisionComponent) -> TilemapLayer?,
	GetTilemap: (self: TilemapCollisionComponent) -> TilemapHandle,
	GetTilemapCollision: (self: TilemapCollisionComponent) -> TilemapCollisionInfo?,
}
export type TilemapCollisionComponentManager = {
	Get: (self: TilemapCollisionComponentManager, entity: Entity) -> TilemapCollisionComponent?,
	Has: (self: TilemapCollisionComponentManager, entity: Entity) -> boolean,
}

export type StaticCollision = {
	GetPhysicsShapeCount: (self: StaticCollision) -> number,
	-- index is 1-based (Luau-style)
	GetPhysicsShape: (self: StaticCollision, index: number) -> PhysicsShape?,
	GetPhysicsShapeLocalPosition: (self: StaticCollision, index: number) -> Vec2,
}
export type StaticCollisionManager = {
	Get: (self: StaticCollisionManager, entity: Entity) -> StaticCollision?,
	Has: (self: StaticCollisionManager, entity: Entity) -> boolean,
	IsEnable: (self: StaticCollisionManager, entity: Entity) -> boolean,
	Enable: (self: StaticCollisionManager, entity: Entity) -> (),
	Disable: (self: StaticCollisionManager, entity: Entity) -> (),
}

export type EventDebugger = {
	SendDebugEvent: (self: EventDebugger, value: number) -> (),
	GetTriggeredCount: (self: EventDebugger) -> number,
}
export type DebugEvent = {
	m_value: number,
}

export type Time = {
	GetElapseTime: (self: Time) -> number,
	GetCurrentTime: (self: Time) -> number,
}

export type Timer = {
	SetInterval: (self: Timer, time: TimeType) -> (),
	GetInterval: (self: Timer) -> TimeType,
	Start: (self: Timer) -> (),
	Pause: (self: Timer) -> (),
	Stop: (self: Timer) -> (),
	Rewind: (self: Timer) -> (),
	SetLoop: (self: Timer, loop: number) -> (),
	SetEventType: (self: Timer, type: TimerEventType) -> (),
	GetEventType: (self: Timer) -> TimerEventType,
	GetID: (self: Timer) -> TimerID,
	IsRunning: (self: Timer) -> boolean,
}

export type TimerManager = {
	Create: (self: TimerManager, interval: TimeType, event_type: TimerEventType, loop: number) -> Timer,
	Remove: (self: TimerManager, id: TimerID) -> (),
	Find: (self: TimerManager, id: TimerID) -> Timer,
}

export type TimerEvent = {
	GetID: (self: TimerEvent) -> TimerID,
	GetEventType: (self: TimerEvent) -> TimerEventType,
}

export type TimerStopEvent = {
	GetID: (self: TimerStopEvent) -> TimerID,
	GetEventType: (self: TimerStopEvent) -> TimerEventType,
}

export type PhysicsShape = {
	GetPosition: (self: PhysicsShape) -> Vec2,
	SetCollisionLayer: (self: PhysicsShape, collision_group: CollisionGroup) -> (),
	GetCollisionLayer: (self: PhysicsShape) -> CollisionGroup,
	SetCollisionMask: (self: PhysicsShape, collision_group: CollisionGroup) -> (),
	GetCollisionMask: (self: PhysicsShape) -> CollisionGroup,
	GetType: (self: PhysicsShape) -> number,
	GetOwner: (self: PhysicsShape) -> Entity,
	MoveTo: (self: PhysicsShape, position: Vec2) -> (),
	Move: (self: PhysicsShape, offset: Vec2) -> (),
	GetStorageType: (self: PhysicsShape) -> number,
	SetQueryEnable: (self: PhysicsShape, enable: boolean) -> (),
	IsQueryEnabled: (self: PhysicsShape) -> boolean,
}

export type CharacterController = {
	MoveAndSlide: (self: CharacterController, dir: Vec2) -> (),
	GetPosition: (self: CharacterController) -> Vec2,
	SetSkin: (self: CharacterController, skin: number) -> (),
	GetSkin: (self: CharacterController) -> number,
	SetMinDisp: (self: CharacterController, disp: number) -> (),
	GetMinDisp: (self: CharacterController) -> number,
	Teleport: (self: CharacterController, pos: Vec2) -> (),
	GetPhysicsShape: (self: CharacterController) -> PhysicsShape?,
}
export type CCTManager = {
	Get: (self: CCTManager, entity: Entity) -> CharacterController?,
	Has: (self: CCTManager, entity: Entity) -> boolean,
}

export type OverlapResult = {
	m_dst_entity: Entity,
	m_dst_shape: PhysicsShape?,
}

export type PhysicsScene = {
	IsEnableDebugDraw: (self: PhysicsScene) -> boolean,
	ToggleDebugDraw: (self: PhysicsScene) -> (),
	Overlap: ((self: PhysicsScene, shape: PhysicsShape, out_result: { OverlapResult }, out_size: number) -> number)
		& ((self: PhysicsScene, lhs: PhysicsShape, rhs: PhysicsShape) -> boolean),
}

export type Trigger = {
	GetEventType: (self: Trigger) -> TriggerEventType,
	SetEventType: (self: Trigger, type: TriggerEventType) -> (),
	EnableTriggerEveryFrameWhenTouch: (self: Trigger, enable: boolean) -> (),
	IsTriggerEveryFrameWhenTouch: (self: Trigger) -> boolean,
	GetTouchingShapes: (self: Trigger) -> { PhysicsShape },
	GetUnderlyingShapes: (self: Trigger) -> { PhysicsShape },
	GetOwner: (self: Trigger) -> Entity,
}

export type TriggerEnterEvent = {
	GetType: (self: TriggerEnterEvent) -> TriggerEventType,
	GetOverlapResult: (self: TriggerEnterEvent) -> OverlapResult,
	GetSrcEntity: (self: TriggerEnterEvent) -> Entity,
}

export type TriggerLeaveEvent = {
	GetType: (self: TriggerLeaveEvent) -> TriggerEventType,
	GetOverlapResult: (self: TriggerLeaveEvent) -> OverlapResult,
	GetSrcEntity: (self: TriggerLeaveEvent) -> Entity,
}

export type TriggerTouchEvent = {
	GetType: (self: TriggerTouchEvent) -> TriggerEventType,
	GetOverlapResult: (self: TriggerTouchEvent) -> OverlapResult,
	GetSrcEntity: (self: TriggerTouchEvent) -> Entity,
}

export type TriggerComponentManager = {
	Get: (self: TriggerComponentManager, entity: Entity) -> Trigger,
	Has: (self: TriggerComponentManager, entity: Entity) -> boolean,
	IsEnable: (self: TriggerComponentManager, entity: Entity) -> boolean,
	Enable: (self: TriggerComponentManager, entity: Entity) -> (),
	Disable: (self: TriggerComponentManager, entity: Entity) -> (),
	RegisterEntity: (self: TriggerComponentManager, entity: Entity, definition: TriggerDefinition) -> (),
}

export type RelationshipManager = {
	Get: (self: RelationshipManager, entity: Entity) -> Relationship?,
	Has: (self: RelationshipManager, entity: Entity) -> boolean,
	RegisterEntity: (self: RelationshipManager, entity: Entity) -> (),
}

export type EntityName = {
	m_name: string,
}
export type EntityNameManager = {
	Get: (self: EntityNameManager, entity: Entity) -> EntityName?,
	Has: (self: EntityNameManager, entity: Entity) -> boolean,
	FindChildByName: (self: EntityNameManager, entity: Entity, name: string) -> Entity,
	FindChildrenByName: (self: EntityNameManager, entity: Entity, name: string) -> { Entity },
}

export type DebugDraw = {
	kOneFrame: TimeType,
	kAlways: TimeType,
	DrawRect: (self: DebugDraw, center: Vec2, half_size: Vec2, color: Color, time: TimeType, use_camera: boolean?) -> (),
	FillRect: (self: DebugDraw, center: Vec2, half_size: Vec2, color: Color, time: TimeType, use_camera: boolean?) -> (),
	DrawCircle: (self: DebugDraw, center: Vec2, radius: number, color: Color, time: TimeType, use_camera: boolean?) -> (),
	AddLine: (self: DebugDraw, p1: Vec2, p2: Vec2, color: Color, time: TimeType, use_camera: boolean?) -> (),
	Clear: (self: DebugDraw) -> (),
}

export type NetAddress = {
	GetIP: (self: NetAddress) -> string,
	m_host: number,
	m_port: number,
}

export type UDPPeer = {
	Disconnect: (self: UDPPeer) -> (),
	GetID: (self: UDPPeer) -> number,
	IsValid: (self: UDPPeer) -> boolean,
	InvalidID: number,
}

export type UDPHost = {
	Connect: (self: UDPHost, addr: NetAddress) -> UDPPeer,
	Send: (self: UDPHost, peer: UDPPeer, net_msg: any, channel_id: number, flags: UDPPacketFlags?) -> (),
	Flush: (self: UDPHost) -> (),
	HandleIncomingNetPacket: (self: UDPHost) -> (),
	GetPeer: (self: UDPHost, id: number) -> UDPPeer,
}

export type EventSystem = {
	AddTimerEvent: (self: EventSystem, cb: (id: EventListenerID, event: TimerEvent) -> ()) -> EventListenerID,
	AddTimerStopEvent: (self: EventSystem, cb: (id: EventListenerID, event: TimerStopEvent) -> ()) -> EventListenerID,
	AddTriggerEnterEvent: (self: EventSystem, cb: (id: EventListenerID, event: TriggerEnterEvent) -> ()) -> EventListenerID,
	AddTriggerLeaveEvent: (self: EventSystem, cb: (id: EventListenerID, event: TriggerLeaveEvent) -> ()) -> EventListenerID,
	AddTriggerTouchEvent: (self: EventSystem, cb: (id: EventListenerID, event: TriggerTouchEvent) -> ()) -> EventListenerID,
	AddDebugEvent: (self: EventSystem, cb: (id: EventListenerID, event: DebugEvent) -> ()) -> EventListenerID,
	AddUIMouseHoverEvent: (self: EventSystem, cb: (id: EventListenerID, event: UIMouseHoverEvent) -> ()) -> EventListenerID,
	AddUIMouseDownEvent: (self: EventSystem, cb: (id: EventListenerID, event: UIMouseDownEvent) -> ()) -> EventListenerID,
	AddUIMouseUpEvent: (self: EventSystem, cb: (id: EventListenerID, event: UIMouseUpEvent) -> ()) -> EventListenerID,
	AddUIMouseClickedEvent: (self: EventSystem, cb: (id: EventListenerID, event: UIMouseClickedEvent) -> ()) -> EventListenerID,
	AddUICheckToggledEvent: (self: EventSystem, cb: (id: EventListenerID, event: UICheckToggledEvent) -> ()) -> EventListenerID,
	AddUIDragEvent: (self: EventSystem, cb: (id: EventListenerID, event: UIDragEvent) -> ()) -> EventListenerID,
	Remove: (self: EventSystem, id: EventListenerID) -> (),
}

export type CommonContext = {
	GetScriptManager: (self: CommonContext) -> ScriptComponentManager,
	GetAssetsManager: (self: CommonContext) -> AssetsManager,
	GetSceneManager: (self: CommonContext) -> SceneManager,
	GetTime: (self: CommonContext) -> Time,
	GetTimerManager: (self: CommonContext) -> TimerManager,
	GetTransformManager: (self: CommonContext) -> TransformManager,
	GetTriggerComponentManager: (self: CommonContext) -> TriggerComponentManager,
	GetRelationshipManager: (self: CommonContext) -> RelationshipManager,
	GetBindPointsComponentManager: (self: CommonContext) -> BindPointsComponentManager,
	GetCCTManager: (self: CommonContext) -> CCTManager,
	GetPhysicsScene: (self: CommonContext) -> PhysicsScene,
	GetStaticCollisionManager: (self: CommonContext) -> StaticCollisionManager,
	GetEventDebugger: (self: CommonContext) -> EventDebugger,
	GetDebugDraw: (self: CommonContext) -> DebugDraw,
	GetTilemapCollisionComponentManager: (self: CommonContext) -> TilemapCollisionComponentManager,
	GetEntityNameManager: (self: CommonContext) -> EntityNameManager,
	GetNetHost: (self: CommonContext) -> UDPHost?,
	GetCommonConfig: (self: CommonContext) -> CommonConfig,
	GetEventSystem: (self: CommonContext) -> EventSystem,
	Log: (self: CommonContext, ...any) -> (),
}

-- Sub-namespace (enum-like) value tables exposed under TL_Common.
export type TilemapLayerTypeNamespace = {
	Tiled: number,
	Object: number,
	Image: number,
}
export type TilemapObjectTypeNamespace = {
	None: number,
	Point: number,
	Circle: number,
	Rect: number,
	Polygon: number,
}
export type UDPPacketFlagNamespace = {
	Reliable: number,
	Unsequenced: number,
	UnreliableFragment: number,
}

-- The runtime global table created by `beginNamespace("TL_Common")`.
declare TL_Common: {
	-- entities / ids
	null_entity: Entity,
	null_timer_id: TimerID,
	null_event_listener_id: EventListenerID,

	-- constructors and statics
	UUID: (() -> UUID) & {
		CreateV4: () -> UUID,
		CreateFromString: (str: string) -> UUID,
	},
	Path: ((path: string) -> Path) & (() -> Path),
	Vec2: ((x: number, y: number) -> Vec2) & (() -> Vec2) & {
		ZERO: Vec2,
		X_UNIT: Vec2,
		Y_UNIT: Vec2,
	},
	Vec2UI: ((x: number, y: number) -> Vec2UI) & (() -> Vec2UI) & {
		ZERO: Vec2UI,
		X_UNIT: Vec2UI,
		Y_UNIT: Vec2UI,
	},
	Color: ((r: number, g: number, b: number, a: number) -> Color) & (() -> Color) & {
		Red: Color,
		Green: Color,
		Blue: Color,
		Black: Color,
		White: Color,
		Yellow: Color,
		Purple: Color,
	},
	Degrees: ((value: number) -> Degrees) & ((radians: Radians) -> Degrees) & (() -> Degrees),
	Radians: ((value: number) -> Radians) & ((degrees: Degrees) -> Radians) & (() -> Radians),
	Transform: () -> Transform,
	Region: () -> Region,
	CollisionGroup: () -> CollisionGroup,
	BindPoints: () -> BindPoints,
	DebugEvent: () -> DebugEvent,
	NetAddress: ((host: number, port: number) -> NetAddress)
		& ((ip: string, port: number) -> NetAddress)
		& (() -> NetAddress),
	UDPPeer: () -> UDPPeer,
	UDPPacketFlags: (...number) -> UDPPacketFlags,

	-- handle constructors (default-constructed, empty handles)
	ImageHandle: () -> ImageHandle,
	SceneHandle: () -> SceneHandle,
	PrefabHandle: () -> PrefabHandle,
	AnimationHandle: () -> AnimationHandle,
	TilemapHandle: () -> TilemapHandle,
	FontHandle: () -> FontHandle,

	-- static class tables (for static member access)
	DebugDraw: {
		kOneFrame: TimeType,
		kAlways: TimeType,
	},

	-- sub-namespaces
	TilemapLayerType: TilemapLayerTypeNamespace,
	TilemapObjectType: TilemapObjectTypeNamespace,
	UDPPacketFlag: UDPPacketFlagNamespace,

	-- free functions
	GetContext: () -> CommonContext,
	GetAngle: (from: Vec2, to: Vec2) -> Radians,
	DecomposeVector: (velocity: Vec2, tangent: Vec2) -> DecompositionResult,
	Rotate: (v: Vec2, degrees: Degrees) -> Vec2,
}
