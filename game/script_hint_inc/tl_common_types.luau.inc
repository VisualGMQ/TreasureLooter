-- Types bound from common/src/scripts/script_binding.cpp (TL.Common)

export type Entity = number
export type TimeType = number
export type TimerID = number

export type UUID = {
	CreateV4: () -> UUID,
	CreateFromString: (str: string) -> UUID,
	IsValid: (self: UUID) -> boolean,
}

export type Vec2 = {
	x: number,
	y: number,
	Length: (self: Vec2) -> number,
	LengthSquared: (self: Vec2) -> number,
	Normalize: (self: Vec2) -> Vec2,
	Dot: (self: Vec2, other: Vec2) -> number,
	Cross: (self: Vec2, other: Vec2) -> number,

	ZERO: Vec2,
	X_UNIT: Vec2,
	Y_UNIT: Vec2 ,
} & ((x: number, y: number) -> Vec2)

export type Vec2UI = {
	x: number,
	y: number,
	Length: (self: Vec2UI) -> number,
	LengthSquared: (self: Vec2UI) -> number,
	Normalize: (self: Vec2UI) -> Vec2UI,
	Dot: (self: Vec2UI, other: Vec2UI) -> number,
	Cross: (self: Vec2UI, other: Vec2UI) -> number,

	ZERO: Vec2UI,
	X_UNIT: Vec2UI,
	Y_UNIT: Vec2UI
} & ((x: number, y: number) -> Vec2UI)

export type Color = {
	r: number, g: number, b: number, a: number,
	Red: (self: Color) -> Color,
	Green: (self: Color) -> Color,
	Blue: (self: Color) -> Color,
	Black: (self: Color) -> Color,
	White: (self: Color) -> Color,
	Yellow: (self: Color) -> Color,
	Purple: (self: Color) -> Color,
} & (() -> Color)

export type DecompositionResult = { m_tangent: Vec2, m_normal: Vec2 }
export type Degrees = { Value: (self: Degrees) -> number } & (number) -> Degrees & (Radians) -> Degrees
export type Radians = { Value: (self: Radians) -> number } & (number) -> Radians & (Degrees) -> Radians

export type Transform = {
	m_position: Vec2,
	m_rotation: Degrees,
	m_size: Vec2,
	m_scale: Vec2,
	GetGlobalPosition: (self: Transform) -> Vec2,
} & (() -> Transform)
export type Region = { m_topleft: Vec2, m_size: Vec2 }

export type Path = {
	parent_path: (self: Path) -> Path,
	filename: (self: Path) -> string,
	extension: (self: Path) -> string,
	has_extension: (self: Path, ext: string?) -> boolean,
	is_absolute: (self: Path) -> boolean,
	is_relative: (self: Path) -> boolean,
	empty: (self: Path) -> boolean,
} & (string) -> Path

export type Image9Grid = {
	left: number,
	right: number,
	top: number,
	bottom: number,
	scale: number,
	IsValid: (self: Image9Grid) -> boolean,
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

export type ImageHandle = { IsValid: (self: ImageHandle) -> boolean, GetFilename: (self: ImageHandle) -> Path?, GetUUID: (self: ImageHandle) -> UUID } & Image
export type FontHandle = { IsValid: (self: FontHandle) -> boolean, GetFilename: (self: FontHandle) -> Path?, GetUUID: (self: FontHandle) -> UUID } & Font
export type ScriptBinaryDataHandle = { IsValid: (self: ScriptBinaryDataHandle) -> boolean, GetFilename: (self: ScriptBinaryDataHandle) -> Path?, GetUUID: (self: ScriptBinaryDataHandle) -> UUID } & ScriptBinaryData

export type PrefabHandle = { IsValid: (self: PrefabHandle) -> boolean, GetFilename: (self: PrefabHandle) -> Path?, GetUUID: (self: PrefabHandle) -> UUID } & Prefab

export type Scene = {
	Instantiate: (self: Scene, prefab: PrefabHandle) -> Entity,
	RemoveEntity: (self: Scene, entity: Entity) -> (),
	GetRootEntity: (self: Scene) -> Entity,
	GetUIRootEntity: (self: Scene) -> Entity,
}

export type SceneHandle = { IsValid: (self: SceneHandle) -> boolean, GetFilename: (self: SceneHandle) -> Path?, GetUUID: (self: SceneHandle) -> UUID } & Scene

export type BindPoint = {
	m_position: Vec2,
	m_name: string,
	GetGlobalPosition: (self: BindPoint) -> Vec2,
}
export type BindPoints = {
	m_bind_points: {},
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
	GetType: (self: TilemapLayer) -> TilemapLayerType,
	AsTiledLayer: (self: TilemapLayer) -> TilemapTileLayer?,
	AsImageLayer: (self: TilemapLayer) -> TilemapImageLayer?,
	AsObjectLayer: (self: TilemapLayer) -> TilemapObjectLayer?,
	GetName: (self: TilemapLayer) -> string,
}

export type TilemapTileLayer = {
	GetTile: (self: TilemapTileLayer, x: number, y: number) -> TilemapLayerTile?,
	GetSize: (self: TilemapTileLayer) -> Vec2,
}

export type TilemapObjectLayer = {
	GetObjectCount: (self: TilemapObjectLayer) -> number,
	GetObject: (self: TilemapObjectLayer, index: number) -> TilemapObject?,
}

export type TilemapImageLayer = {
	GetImage: (self: TilemapImageLayer) -> ImageHandle?,
	GetPosition: (self: TilemapImageLayer) -> Vec2,
}

export type TilemapObject = {
	GetType: (self: TilemapObject) -> TilemapObjectType,
	IsVisiable: (self: TilemapObject) -> boolean,
	AsCircle: (self: TilemapObject) -> Circle?,
	AsRect: (self: TilemapObject) -> Rect?,
	AsPolygon: (self: TilemapObject) -> Polygon?,
	AsPoint: (self: TilemapObject) -> Vec2?,
}

export type TilemapLayerType = {
	Tiled: number,
	Object: number,
	Image: number,
}

export type TilemapObjectType = {
	None: number,
	Point: number,
	Circle: number,
	Rect: number,
	Polygon: number,
}

export type Tilemap = {
	GetLayerCount: (self: Tilemap) -> number,
	GetLayer: (self: Tilemap, index: number) -> TilemapLayer?,
	GetTilesetCount: (self: Tilemap) -> number,
	GetTileset: (self: Tilemap, index: number) -> Tileset?,
	GetTile: (self: Tilemap, gid: number) -> Tile?,
	GetTileSize: (self: Tilemap) -> Vec2,
	GetFilename: (self: Tilemap) -> string,
}

export type AnimationHandle = { IsValid: (self: AnimationHandle) -> boolean, GetFilename: (self: AnimationHandle) -> Path?, GetUUID: (self: AnimationHandle) -> UUID } & Animation
export type TilemapHandle = { IsValid: (self: TilemapHandle) -> boolean, GetFilename: (self: TilemapHandle) -> Path?, GetUUID: (self: TilemapHandle) -> UUID } & Tilemap

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
	Get: (self: ScriptComponentManager, entity: Entity) -> unknown,
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
	GetFontManager: (self: AssetsManager) -> FontManager,

	-- from schema
	GetPrefabManager: (self: AssetsManager) -> PrefabAssetManager,
	GetGameConfigManager: (self: AssetsManager) -> GameConfigAssetManager,
	GetWeaponDefinitionManager: (self: AssetsManager) -> WeaponDefinitionAssetManager,
	GetCharacterDefinitionManager: (self: AssetsManager) -> CharacterDefinitionAssetManager,
	GetLevelDefinitionManager: (self: AssetsManager) -> LevelDefinitionAssetManager,
	GetInputConfigManager: (self: AssetsManager) -> InputConfigAssetManager,
	GetSceneDefinitionManager: (self: AssetsManager) -> SceneDefinitionAssetManager,
	GetPhysicsShapeDefinitionManager: (self: AssetsManager) -> PhysicsShapeDefinitionAssetManager,
	GetUIWidgetDefinitionManager: (self: AssetsManager) -> UIWidgetDefinitionAssetManager,
	GetItemDefinitionManager: (self: AssetsManager) -> ItemDefinitionAssetManager,
}

export type TilemapCollisionInfo = { m_topleft: Vec2 }

export type TilemapCollisionComponent = {
	GetLayer: (self: TilemapCollisionComponent) -> unknown,
	GetTilemap: (self: TilemapCollisionComponent) -> TilemapHandle,
	GetTilemapCollision: (self: TilemapCollisionComponent) -> TilemapCollisionInfo?,
}
export type TilemapCollisionComponentManager = { Get: (self: TilemapCollisionComponentManager, entity: Entity) -> TilemapCollisionComponent?, Has: (self: TilemapCollisionComponentManager, entity: Entity) -> boolean }

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
export type TriggerComponentManager = {
	Get: (self: TriggerComponentManager, entity: Entity) -> Trigger,
	Has: (self: TriggerComponentManager, entity: Entity) -> boolean,
	IsEnable: (self: TriggerComponentManager, entity: Entity) -> boolean,
	Enable: (self: TriggerComponentManager, entity: Entity) -> (),
	Disable: (self: TriggerComponentManager, entity: Entity) -> (),
	RegisterEntity: (self: TriggerComponentManager, entity: Entity, definition: TriggerDefinition) -> (),
}

export type EventDebugger = {
	SendDebugEvent: (self: EventDebugger, value: number) -> (),
	GetTriggeredCount: (self: EventDebugger) -> number,
}
export type DebugEvent = { m_value: number } & (() -> DebugEvent)

export type Time = { GetElapseTime: (self: Time) -> number, GetCurrentTime: (self: Time) -> number }

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
}

export type CharacterController = {
	MoveAndSlide: (self: CharacterController, dir: Vec2) -> (),
	GetPosition: (self: CharacterController) -> Vec2,
	SetSkin: (self: CharacterController, skin: unknown) -> (),
	GetSkin: (self: CharacterController) -> unknown,
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
	Overlap: ((self: PhysicsScene, shape: PhysicsShape, out_result: {OverlapResult}, out_size: number) -> number)
		& ((self: PhysicsScene, lhs: PhysicsShape, rhs: PhysicsShape) -> boolean),
}

export type Trigger = {
	GetEventType: (self: Trigger) -> TriggerEventType,
	SetEventType: (self: Trigger, type: TriggerEventType) -> (),
	EnableTriggerEveryFrameWhenTouch: (self: Trigger, enable: boolean) -> (),
	IsTriggerEveryFrameWhenTouch: (self: Trigger) -> boolean,
	GetTouchingShapes: (self: Trigger) -> {PhysicsShape},
	GetUnderlyingShapes: (self: Trigger) -> {PhysicsShape},
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

export type RelationshipManager = {
	Get: (self: RelationshipManager, entity: Entity) -> Relationship?,
	Has: (self: RelationshipManager, entity: Entity) -> boolean,
	RegisterEntity: (self: RelationshipManager, entity: Entity) -> (),
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

export type GetAngle = (from: { x: number, y: number }, to: { x: number, y: number }) -> number
export type DecomposeVector = (velocity: Vec2, tangent: Vec2) -> DecompositionResult
