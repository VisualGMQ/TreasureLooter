export type Entity = number

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
}

export type DecompositionResult = { m_tangent: Vec2, m_normal: Vec2 }
export type Degrees = { Value: (self: Degrees) -> number } & (number) -> Degrees & (Radians) -> Degrees
export type Radians = { Value: (self: Radians) -> number } & (number) -> Radians & (Degrees) -> Radians

export type Transform = {
	m_position: Vec2,
	m_rotation: number,
	m_size: Vec2,
	m_scale: Vec2,
}
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

export type Rect = { center: Vec2, half_size: Vec2 }
export type Circle = { center: Vec2, radius: number }

export type Image = {
	GetSize: (self: Image) -> Vec2,
	ChangeColorMask: (self: Image, color: Color) -> (),
} & (r: number, g: number, b: number, a: number) -> Color

export type Level = {
	Instantiate: (self: Level, prefab: PrefabHandle) -> Entity,
	RemoveEntity: (self: Level, entity: Entity) -> (),
	GetRootEntity: (self: Level) -> Entity,
	GetUIRootEntity: (self: Level) -> Entity,
}

export type ImageHandle = { IsValid: (self: ImageHandle) -> boolean } & Image
export type LevelHandle = { IsValid: (self: LevelHandle) -> boolean } & Level
export type PrefabHandle = { IsValid: (self: PrefabHandle) -> boolean }
export type AnimationHandle = { IsValid: (self: AnimationHandle) -> boolean }
export type TilemapHandle = { IsValid: (self: TilemapHandle) -> boolean }
export type FontHandle = { IsValid: (self: FontHandle) -> boolean }
export type ScriptBinaryDataHandle = { IsValid: (self: ScriptBinaryDataHandle) -> boolean }

export type GameplayConfig = {
	sprite_sheet: ImageHandle?,
	faceset: ImageHandle?,
	speed: number,
	move_left_animation: AnimationHandle?,
	move_right_animation: AnimationHandle?,
	move_up_animation: AnimationHandle?,
	move_down_animation: AnimationHandle?,
	weapon_entity: number?,
}

export type Sprite = {
	m_region: Region,
	m_size: Vec2,
	m_anchor: Vec2,
	m_z_order: number,
	m_color: Color,
	m_flip: number,
}

export type Relationship = { m_children: { Entity } }

export type ScriptBinaryDataManager = {
	Load: (self: ScriptBinaryDataManager, path: string, force: boolean?) -> ScriptBinaryDataHandle,
	Find: (self: ScriptBinaryDataManager, path: string) -> ScriptBinaryDataHandle?,
}
export type ScriptComponentManager = { Get: (self: ScriptComponentManager, entity: Entity) -> unknown }
export type TransformManager = {
	Get: (self: TransformManager, entity: Entity) -> Transform?,
	Has: (self: TransformManager, entity: Entity) -> boolean,
}
export type ImageManager = {
	Load: (self: ImageManager, path: string, force: boolean?) -> ImageHandle,
	Find: (self: ImageManager, path: string) -> ImageHandle?,
}
export type TilemapManager = {
	Load: (self: TilemapManager, path: string, force: boolean?) -> TilemapHandle,
	Find: (self: TilemapManager, path: string) -> TilemapHandle?,
}
export type AnimationManager = {
	Load: (self: AnimationManager, path: string, force: boolean?) -> AnimationHandle,
	Find: (self: AnimationManager, path: string) -> AnimationHandle?,
}
export type LevelManager = {
	Load: (self: LevelManager, path: string, force: boolean?) -> LevelHandle,
	Find: (self: LevelManager, path: string) -> LevelHandle?,
	GetCurrentLevel: (self: LevelManager) -> Level?,
	Switch: (self: LevelManager, level: LevelHandle) -> (),
}
export type PrefabManager = { Find: (self: PrefabManager, path: string) -> PrefabHandle? }
export type FontManager = {
	Load: (self: FontManager, path: string, force: boolean?) -> FontHandle,
	Find: (self: FontManager, path: string) -> FontHandle?,
}
export type AssetsManager = {
	GetScriptBinaryDataManager: (self: AssetsManager) -> ScriptBinaryDataManager,
	GetImageManager: (self: AssetsManager) -> ImageManager,
	GetTilemapManager: (self: AssetsManager) -> TilemapManager,
	GetAnimationManager: (self: AssetsManager) -> AnimationManager,
	GetLevelManager: (self: AssetsManager) -> LevelManager,
	GetPrefabManager: (self: AssetsManager) -> PrefabManager,
	GetFontManager: (self: AssetsManager) -> FontManager,
}

export type Renderer = {}
export type Window = {}
export type TriggerComponentManager = { Get: (self: TriggerComponentManager, entity: Entity) -> unknown, Has: (self: TriggerComponentManager, entity: Entity) -> boolean }
export type UIComponentManager = { Get: (self: UIComponentManager, entity: Entity) -> unknown, Has: (self: UIComponentManager, entity: Entity) -> boolean }
export type TilemapComponentManager = { Get: (self: TilemapComponentManager, entity: Entity) -> unknown, Has: (self: TilemapComponentManager, entity: Entity) -> boolean }

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
export type Time = { GetElapseTime: (self: Time) -> number, GetCurrentTime: (self: Time) -> number }
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
	RegisterEntity: (self: SpriteManager, entity: Entity, def: Sprite) -> (),
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
}
export type CharacterController = {
	MoveAndSlide: (self: CharacterController, dir: Vec2) -> (),
	GetPosition: (self: CharacterController) -> Vec2,
	SetSkin: (self: CharacterController, skin: unknown) -> (),
	GetSkin: (self: CharacterController) -> unknown,
	SetMinDisp: (self: CharacterController, disp: number) -> (),
	GetMinDisp: (self: CharacterController) -> number,
	Teleport: (self: CharacterController, pos: Vec2) -> (),
}
export type CCTManager = {
	Get: (self: CCTManager, entity: Entity) -> CharacterController?,
	Has: (self: CCTManager, entity: Entity) -> boolean,
}
export type GameplayConfigManager = {
	Get: (self: GameplayConfigManager, entity: Entity) -> GameplayConfig?,
	Has: (self: GameplayConfigManager, entity: Entity) -> boolean,
	RegisterEntity: (self: GameplayConfigManager, entity: Entity, config: GameplayConfig) -> (),
}
export type RelationshipManager = {
	Get: (self: RelationshipManager, entity: Entity) -> Relationship?,
	Has: (self: RelationshipManager, entity: Entity) -> boolean,
	RegisterEntity: (self: RelationshipManager, entity: Entity) -> (),
}

export type GameContext = {
	GetCamera: (self: GameContext) -> Camera,
	GetScriptManager: (self: GameContext) -> ScriptComponentManager,
	GetAssetsManager: (self: GameContext) -> AssetsManager,
	GetLevelManager: (self: GameContext) -> LevelManager,
	GetTime: (self: GameContext) -> Time,
	GetTransformManager: (self: GameContext) -> TransformManager,
	GetSpriteManager: (self: GameContext) -> SpriteManager,
	GetRenderer: (self: GameContext) -> Renderer,
	GetWindow: (self: GameContext) -> Window,
	GetInputManager: (self: GameContext) -> InputManager,
	GetTriggerComponentManager: (self: GameContext) -> TriggerComponentManager,
	GetRelationshipManager: (self: GameContext) -> RelationshipManager,
	GetAnimationPlayerManager: (self: GameContext) -> AnimationPlayerManager,
	GetUIManager: (self: GameContext) -> UIComponentManager,
	GetGameplayConfigManager: (self: GameContext) -> GameplayConfigManager,
	GetTilemapComponentManager: (self: GameContext) -> TilemapComponentManager,
	GetCCTManager: (self: GameContext) -> CCTManager,
}

export type GetZOrderByYSorting = (y: number, layer: number) -> number
export type GetAngle = (from: { x: number, y: number }, to: { x: number, y: number }) -> number
export type DecomposeVector = (velocity: Vec2, tangent: Vec2) -> DecompositionResult
export type GetContext = () -> GameContext

local _M = setmetatable({}, TL)

return _M