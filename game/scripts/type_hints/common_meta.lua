---@meta
-- LuaLS (LuaCATS) definition file for the `TL_Common` C++ binding namespace.
-- Hand-written equivalent of common/src/scripts/script_binding.cpp.
-- Loaded through LuaLS `workspace.library`, so every type below is a global
-- type and `TL_Common` is a global value (no `require` needed).
-- Types declared in schema_meta.lua / proto_meta.lua are referenced as globals.

---@alias Entity number
---@alias TimeType number
---@alias TimerID number
---@alias EventListenerID number
---@alias TriggerID number

-- Opaque flags wrapper produced by `TL_Common.UDPPacketFlags(...)`.
---@alias UDPPacketFlags number

---@class UUID
---@field IsValid fun(self: UUID): boolean

---@class UUIDCtor
---@field CreateV4 fun(): UUID
---@field CreateFromString fun(str: string): UUID
---@overload fun():UUID

---@class Vec2
---@field x number
---@field y number
---@field Length fun(self: Vec2): number
---@field LengthSquared fun(self: Vec2): number
---@field Normalize fun(self: Vec2): Vec2
---@field Dot fun(self: Vec2, other: Vec2): number
---@field Cross fun(self: Vec2, other: Vec2): number
---@operator add(Vec2): Vec2
---@operator sub(Vec2): Vec2
---@operator mul(Vec2|number): Vec2
---@operator div(Vec2|number): Vec2

---@class Vec2Ctor
---@field ZERO Vec2
---@field X_UNIT Vec2
---@field Y_UNIT Vec2
---@overload fun():Vec2
---@overload fun(number): Vec2
---@overload fun(number, number): Vec2

---@class Vec2UI
---@field x number
---@field y number
---@field Length fun(self: Vec2UI): number
---@field LengthSquared fun(self: Vec2UI): number
---@field Normalize fun(self: Vec2UI): Vec2UI
---@field Dot fun(self: Vec2UI, other: Vec2UI): number
---@field Cross fun(self: Vec2UI, other: Vec2UI): number
---@operator add(Vec2UI): Vec2UI
---@operator sub(Vec2UI): Vec2UI
---@operator mul(Vec2UI|number): Vec2UI
---@operator div(Vec2UI|number): Vec2UI

---@class Vec2UICtor
---@field ZERO Vec2UI
---@field X_UNIT Vec2UI
---@field Y_UNIT Vec2UI
---@overload fun():Vec2UI
---@overload fun(number): Vec2UI
---@overload fun(number, number): Vec2UI

---@class Color
---@field r number
---@field g number
---@field b number
---@field a number

---@class ColorCtor
---@field Red Color
---@field Green Color
---@field Blue Color
---@field Black Color
---@field White Color
---@field Yellow Color
---@field Purple Color
---@overload fun():Color
---@overload fun(number, number, number, number): Color

---@class DecompositionResult
---@field m_tangent Vec2
---@field m_normal Vec2

---@class Degrees
---@field Value fun(self: Degrees): number
---@operator add(Degrees): Degrees
---@operator sub(Degrees): Degrees
---@operator mul(number): Degrees
---@operator div(number): Degrees

---@class DegreesCtor
---@operator call(number|Radians): Degrees

---@class Radians
---@field Value fun(self: Radians): number
---@operator add(Radians): Radians
---@operator sub(Radians): Radians
---@operator mul(number): Radians
---@operator div(number): Radians

---@class RadiansCtor
---@operator call(number|Degrees): Radians

---@class Transform
---@field m_position Vec2
---@field m_rotation Degrees
---@field m_size Vec2
---@field m_scale Vec2
---@field GetGlobalPosition fun(self: Transform): Vec2

---@class Region
---@field m_topleft Vec2
---@field m_size Vec2

---@class Path
---@field parent_path fun(self: Path): Path
---@field filename fun(self: Path): string
---@field extension fun(self: Path): string
---@field has_extension fun(self: Path, ext: string?): boolean
---@field is_absolute fun(self: Path): boolean
---@field is_relative fun(self: Path): boolean
---@field empty fun(self: Path): boolean
---@field string fun(self: Path): string

---@class PathCtor
---@overload fun():Path
---@overload fun(string): Path

---@class CollisionGroup
---@field Add fun(self: CollisionGroup, group: CollisionGroupType)
---@field Remove fun(self: CollisionGroup, group: CollisionGroupType)
---@field Has fun(self: CollisionGroup, group: CollisionGroupType): boolean
---@field Clear fun(self: CollisionGroup)
---@field CanCollision fun(self: CollisionGroup, o: CollisionGroup): boolean
---@field GetUnderlying fun(self: CollisionGroup): number
---@field SetUnderlying fun(self: CollisionGroup, underlying: number)

---@class Image
---@field GetSize fun(self: Image): Vec2
---@field ChangeColorMask fun(self: Image, color: Color)

---@class ScriptBinaryData

---@class Animation
---@field GetFinishTime fun(self: Animation): TimeType

---@class Font
---@field IsValid fun(self: Font): boolean
---@field GetHeight fun(self: Font): number
---@field SetFontSize fun(self: Font, size: number)

---@class ImageHandle : Image
---@field IsValid fun(self: ImageHandle): boolean
---@field GetFilename fun(self: ImageHandle): Path?
---@field GetUUID fun(self: ImageHandle): UUID

---@class FontHandle : Font
---@field IsValid fun(self: FontHandle): boolean
---@field GetFilename fun(self: FontHandle): Path?
---@field GetUUID fun(self: FontHandle): UUID

---@class ScriptBinaryDataHandle : ScriptBinaryData
---@field IsValid fun(self: ScriptBinaryDataHandle): boolean
---@field GetFilename fun(self: ScriptBinaryDataHandle): Path?
---@field GetUUID fun(self: ScriptBinaryDataHandle): UUID

---@class AnimationHandle : Animation
---@field IsValid fun(self: AnimationHandle): boolean
---@field GetFilename fun(self: AnimationHandle): Path?
---@field GetUUID fun(self: AnimationHandle): UUID

---@class Scene
---@field Instantiate fun(self: Scene, prefab: PrefabHandle, transform: Transform?): Entity
---@field RemoveEntity fun(self: Scene, entity: Entity)
---@field GetRootEntity fun(self: Scene): Entity
---@field GetUIRootEntity fun(self: Scene): Entity

---@class TilemapHandle : Tilemap
---@field IsValid fun(self: TilemapHandle): boolean
---@field GetFilename fun(self: TilemapHandle): Path?
---@field GetUUID fun(self: TilemapHandle): UUID

---@class SceneHandle : Scene
---@field IsValid fun(self: SceneHandle): boolean
---@field GetFilename fun(self: SceneHandle): Path?
---@field GetUUID fun(self: SceneHandle): UUID

---@class BindPoint
---@field m_position Vec2
---@field m_name string
---@field GetGlobalPosition fun(self: BindPoint): Vec2

---@class BindPoints
---@field m_bind_points { [string]: BindPoint }

---@class BindPointsComponentManager
---@field Get fun(self: BindPointsComponentManager, entity: Entity): BindPoints
---@field Has fun(self: BindPointsComponentManager, entity: Entity): boolean
---@field ToggleDebugDraw fun(self: BindPointsComponentManager)

---@class Tile
---@field m_image ImageHandle
---@field m_region Region
---@field m_id number
---@field m_tile_size Vec2

---@class TilemapLayerTile
---@field m_gid number
---@field GetFlipValue fun(self: TilemapLayerTile): number

---@class Tileset
---@field GetTile fun(self: Tileset, gid: number): Tile?
---@field HasTile fun(self: Tileset, gid: number): boolean
---@field GetTileSize fun(self: Tileset): Vec2

---@class TilemapLayer
---@field GetType fun(self: TilemapLayer): number
---@field AsTiledLayer fun(self: TilemapLayer): TilemapTileLayer?
---@field AsImageLayer fun(self: TilemapLayer): TilemapImageLayer?
---@field AsObjectLayer fun(self: TilemapLayer): TilemapObjectLayer?
---@field GetName fun(self: TilemapLayer): string

---@class TilemapTileLayer : TilemapLayer
---@field GetTile fun(self: TilemapTileLayer, x: number, y: number): TilemapLayerTile?
---@field GetSize fun(self: TilemapTileLayer): Vec2

---@class TilemapObjectLayer : TilemapLayer
---@field GetObjectCount fun(self: TilemapObjectLayer): number
---@field GetObject fun(self: TilemapObjectLayer, index: number): TilemapObject?

---@class TilemapImageLayer : TilemapLayer
---@field GetImage fun(self: TilemapImageLayer): ImageHandle?
---@field GetPosition fun(self: TilemapImageLayer): Vec2

---@class TilemapObject
---@field GetType fun(self: TilemapObject): number
---@field GetName fun(self: TilemapObject): string
---@field IsVisiable fun(self: TilemapObject): boolean
---@field AsCircle fun(self: TilemapObject): Circle?
---@field AsRect fun(self: TilemapObject): Rect?
---@field AsPolygon fun(self: TilemapObject): Polygon?
---@field AsPoint fun(self: TilemapObject): Vec2?

---@class TilemapCollisionInfo
---@field m_topleft Vec2

---@class Tilemap
---@field GetLayerCount fun(self: Tilemap): number
---@field GetLayer fun(self: Tilemap, index: number): TilemapLayer?
---@field GetTilesetCount fun(self: Tilemap): number
---@field GetTileset fun(self: Tilemap, index: number): Tileset?
---@field GetTile fun(self: Tilemap, gid: number): Tile?
---@field GetTileSize fun(self: Tilemap): Vec2
---@field GetFilename fun(self: Tilemap): string

---@class Relationship
---@field AddChild fun(self: Relationship, entity: Entity)
---@field Get fun(self: Relationship, index: number): Entity
---@field GetChildrenCount fun(self: Relationship): number
---@field GetParent fun(self: Relationship): Entity
---@field RemoveChild fun(self: Relationship, entity: Entity)
---@field RemoveFromParent fun(self: Relationship)

---@class ScriptBinaryDataManager
---@field Load fun(self: ScriptBinaryDataManager, path: Path|string, force: boolean?): ScriptBinaryDataHandle
---@field Find fun(self: ScriptBinaryDataManager, path: string): ScriptBinaryDataHandle

---@class ScriptComponentManager
---@field Get fun(self: ScriptComponentManager, entity: Entity): any
---@field Has fun(self: ScriptComponentManager, entity: Entity): boolean
---@field IsEnable fun(self: ScriptComponentManager, entity: Entity): boolean
---@field SubscribeEvent fun(self: ScriptComponentManager, entity: Entity, eventName: string)

---@class TransformManager
---@field Get fun(self: TransformManager, entity: Entity): Transform?
---@field Has fun(self: TransformManager, entity: Entity): boolean

---@class ImageManager
---@field Load fun(self: ImageManager, path: Path|string, force: boolean?): ImageHandle
---@field Find fun(self: ImageManager, path: string): ImageHandle

---@class TilemapManager
---@field Load fun(self: TilemapManager, path: Path|string, force: boolean?): TilemapHandle
---@field Find fun(self: TilemapManager, path: string): TilemapHandle

---@class AnimationManager
---@field Load fun(self: AnimationManager, path: Path|string, force: boolean?): AnimationHandle
---@field Find fun(self: AnimationManager, path: string): AnimationHandle

---@class SceneManager
---@field Load fun(self: SceneManager, path: Path|string, force: boolean?): SceneHandle
---@field Find fun(self: SceneManager, path: string): SceneHandle
---@field Create fun(self: SceneManager, definition: SceneDefinitionHandle): SceneHandle
---@field Unload fun(self: SceneManager, handle: SceneHandle)
---@field GetCurrentScene fun(self: SceneManager): Scene?
---@field Switch fun(self: SceneManager, scene: SceneHandle)

---@class FontManager
---@field Load fun(self: FontManager, path: Path|string, force: boolean?): FontHandle
---@field Find fun(self: FontManager, path: string): FontHandle

---@class AssetsManager
---@field GetScriptBinaryDataManager fun(self: AssetsManager): ScriptBinaryDataManager
---@field GetImageManager fun(self: AssetsManager): ImageManager
---@field GetTilemapManager fun(self: AssetsManager): TilemapManager
---@field GetAnimationManager fun(self: AssetsManager): AnimationManager
---@field GetSceneDefinitionManager fun(self: AssetsManager): SceneDefinitionAssetManager
---@field GetFontManager fun(self: AssetsManager): FontManager
-- registered by the schema binding (see schema_meta.lua)
---@field GetPrefabManager fun(self: AssetsManager): PrefabAssetManager
---@field GetCharacterDefinitionManager fun(self: AssetsManager): CharacterDefinitionAssetManager
---@field GetLevelDefinitionManager fun(self: AssetsManager): LevelDefinitionAssetManager
---@field GetInputConfigManager fun(self: AssetsManager): InputConfigAssetManager
---@field GetPhysicsShapeDefinitionManager fun(self: AssetsManager): PhysicsShapeDefinitionAssetManager
---@field GetUIWidgetDefinitionManager fun(self: AssetsManager): UIWidgetDefinitionAssetManager
---@field GetItemDefinitionManager fun(self: AssetsManager): ItemDefinitionAssetManager
---@field GetObjectDefinitionTableManager fun(self: AssetsManager): ObjectDefinitionTableAssetManager
---@field GetFXDefinitionManager fun(self: AssetsManager): FXDefinitionAssetManager
---@field GetSkillDefinitionManager fun(self: AssetsManager): SkillDefinitionAssetManager
---@field GetScriptHFSMDefinitionManager fun(self: AssetsManager): ScriptHFSMDefinitionAssetManager

---@class TilemapCollisionComponent
---@field GetLayer fun(self: TilemapCollisionComponent): TilemapLayer?
---@field GetTilemap fun(self: TilemapCollisionComponent): TilemapHandle
---@field GetTilemapCollision fun(self: TilemapCollisionComponent): TilemapCollisionInfo?

---@class TilemapCollisionComponentManager
---@field Get fun(self: TilemapCollisionComponentManager, entity: Entity): TilemapCollisionComponent?
---@field Has fun(self: TilemapCollisionComponentManager, entity: Entity): boolean

---@class StaticCollision
---@field GetPhysicsShapeCount fun(self: StaticCollision): number
-- index is 1-based (Lua-style)
---@field GetPhysicsShape fun(self: StaticCollision, index: number): PhysicsShape?
---@field GetPhysicsShapeLocalPosition fun(self: StaticCollision, index: number): Vec2

---@class StaticCollisionManager
---@field Get fun(self: StaticCollisionManager, entity: Entity): StaticCollision?
---@field Has fun(self: StaticCollisionManager, entity: Entity): boolean
---@field IsEnable fun(self: StaticCollisionManager, entity: Entity): boolean
---@field Enable fun(self: StaticCollisionManager, entity: Entity)
---@field Disable fun(self: StaticCollisionManager, entity: Entity)

---@class EventDebugger
---@field SendDebugEvent fun(self: EventDebugger, value: number)
---@field GetTriggeredCount fun(self: EventDebugger): number

---@class DebugEvent
---@field m_value number

---@class Time
---@field GetElapseTime fun(self: Time): number
---@field GetCurrentTime fun(self: Time): number

---@class Timer
---@field SetInterval fun(self: Timer, time: TimeType)
---@field GetInterval fun(self: Timer): TimeType
---@field SetTimerListener fun(self: Timer, cb: fun(self: TimerEvent))
---@field SetTimerStopListener fun(self: Timer, cb: fun(self: TimerStopEvent))
---@field Start fun(self: Timer)
---@field Pause fun(self: Timer)
---@field Stop fun(self: Timer)
---@field Rewind fun(self: Timer)
---@field SetLoop fun(self: Timer, loop: number)
---@field SetEventType fun(self: Timer, type: TimerEventType)
---@field GetEventType fun(self: Timer): TimerEventType
---@field GetID fun(self: Timer): TimerID
---@field IsRunning fun(self: Timer): boolean

---@class TimerManager
---@field Create fun(self: TimerManager, interval: TimeType, event_type: TimerEventType, loop: number): Timer
---@field Remove fun(self: TimerManager, id: TimerID)
---@field Find fun(self: TimerManager, id: TimerID): Timer

---@class TimerEvent
---@field GetTimer fun(self: TimerEvent): Timer
---@field GetEventType fun(self: TimerEvent): TimerEventType

---@class TimerStopEvent
---@field GetTimer fun(self: TimerStopEvent): Timer
---@field GetEventType fun(self: TimerStopEvent): TimerEventType

---@class PhysicsShape
---@field GetPosition fun(self: PhysicsShape): Vec2
---@field SetCollisionLayer fun(self: PhysicsShape, collision_group: CollisionGroup)
---@field GetCollisionLayer fun(self: PhysicsShape): CollisionGroup
---@field SetCollisionMask fun(self: PhysicsShape, collision_group: CollisionGroup)
---@field GetCollisionMask fun(self: PhysicsShape): CollisionGroup
---@field GetType fun(self: PhysicsShape): number
---@field GetOwner fun(self: PhysicsShape): Entity
---@field MoveTo fun(self: PhysicsShape, position: Vec2)
---@field Move fun(self: PhysicsShape, offset: Vec2)
---@field GetStorageType fun(self: PhysicsShape): number
---@field SetQueryEnable fun(self: PhysicsShape, enable: boolean)
---@field IsQueryEnabled fun(self: PhysicsShape): boolean

---@class CharacterController
---@field MoveAndSlide fun(self: CharacterController, dir: Vec2)
---@field GetPosition fun(self: CharacterController): Vec2
---@field SetSkin fun(self: CharacterController, skin: number)
---@field GetSkin fun(self: CharacterController): number
---@field SetMinDisp fun(self: CharacterController, disp: number)
---@field GetMinDisp fun(self: CharacterController): number
---@field Teleport fun(self: CharacterController, pos: Vec2)
---@field GetPhysicsShape fun(self: CharacterController): PhysicsShape?

---@class CCTManager
---@field Get fun(self: CCTManager, entity: Entity): CharacterController?
---@field Has fun(self: CCTManager, entity: Entity): boolean
---@field Enable fun(self: CCTManager, entity: Entity)
---@field Disable fun(self: CCTManager, entity: Entity)

---@class OverlapResult
---@field m_dst_entity Entity
---@field m_dst_shape PhysicsShape?

---@class PhysicsScene
---@field IsEnableDebugDraw fun(self: PhysicsScene): boolean
---@field ToggleDebugDraw fun(self: PhysicsScene)
---@field RenderShape fun(self: PhysicsScene, shape: PhysicsShape, color: Color)
-- `Overlap` is overloaded in C++; kept loose here.
---@field Overlap fun(self: PhysicsScene, ...: any): any

---@class Trigger
---@field GetEventType fun(self: Trigger): TriggerEventType
---@field SetEventType fun(self: Trigger, type: TriggerEventType)
---@field EnableTriggerEveryFrameWhenTouch fun(self: Trigger, enable: boolean)
---@field IsTriggerEveryFrameWhenTouch fun(self: Trigger): boolean
---@field GetTouchingShapes fun(self: Trigger): PhysicsShape[]
---@field GetUnderlyingShapes fun(self: Trigger): PhysicsShape[]
---@field GetOwner fun(self: Trigger): Entity
---@field SetEnterListener fun(self: Trigger, cb: fun(self: TriggerEnterEvent))
---@field SetLeaveListener fun(self: Trigger, cb: fun(self: TriggerLeaveEvent))
---@field SetTouchListener fun(self: Trigger, cb: fun(self: TriggerTouchEvent))
---@field GetID fun(self: Trigger): TriggerID
---@field MoveTo fun(self: Trigger, transform: Transform)
---@field Enable fun(self: Trigger)
---@field Disable fun(self: Trigger)
---@field Update fun(self: Trigger)

---@class TriggerEnterEvent
---@field GetType fun(self: TriggerEnterEvent): TriggerEventType
---@field GetOverlapResult fun(self: TriggerEnterEvent): OverlapResult
---@field GetSrcEntity fun(self: TriggerEnterEvent): Entity

---@class TriggerLeaveEvent
---@field GetType fun(self: TriggerLeaveEvent): TriggerEventType
---@field GetOverlapResult fun(self: TriggerLeaveEvent): OverlapResult
---@field GetSrcEntity fun(self: TriggerLeaveEvent): Entity

---@class TriggerTouchEvent
---@field GetType fun(self: TriggerTouchEvent): TriggerEventType
---@field GetOverlapResult fun(self: TriggerTouchEvent): OverlapResult
---@field GetSrcEntity fun(self: TriggerTouchEvent): Entity

---@class TriggerComponentManager
---@field Get fun(self: TriggerComponentManager, entity: Entity): Trigger
---@field Has fun(self: TriggerComponentManager, entity: Entity): boolean
---@field IsEnable fun(self: TriggerComponentManager, entity: Entity): boolean
---@field Enable fun(self: TriggerComponentManager, entity: Entity)
---@field Disable fun(self: TriggerComponentManager, entity: Entity)
---@field RegisterEntity fun(self: TriggerComponentManager, entity: Entity, definition: TriggerDefinition)

---@class RelationshipManager
---@field Get fun(self: RelationshipManager, entity: Entity): Relationship?
---@field Has fun(self: RelationshipManager, entity: Entity): boolean
---@field RegisterEntity fun(self: RelationshipManager, entity: Entity)

---@class EntityName
---@field m_name string

---@class EntityNameManager
---@field Get fun(self: EntityNameManager, entity: Entity): EntityName?
---@field Has fun(self: EntityNameManager, entity: Entity): boolean
---@field FindChildByName fun(self: EntityNameManager, entity: Entity, name: string): Entity
---@field FindChildrenByName fun(self: EntityNameManager, entity: Entity, name: string): Entity[]

---@class DebugDraw
---@field kOneFrame TimeType
---@field kAlways TimeType
---@field DrawRect fun(self: DebugDraw, center: Vec2, half_size: Vec2, color: Color, time: TimeType, use_camera: boolean?)
---@field FillRect fun(self: DebugDraw, center: Vec2, half_size: Vec2, color: Color, time: TimeType, use_camera: boolean?)
---@field DrawCircle fun(self: DebugDraw, center: Vec2, radius: number, color: Color, time: TimeType, use_camera: boolean?)
---@field AddLine fun(self: DebugDraw, p1: Vec2, p2: Vec2, color: Color, time: TimeType, use_camera: boolean?)
---@field Clear fun(self: DebugDraw)

---@class NetAddress
---@field GetIP fun(self: NetAddress): string
---@field m_host number
---@field m_port number

---@class NetAddressCtor
---@overload fun():NetAddress
---@overload fun(string, number): NetAddress

---@class UDPPeer
---@field Disconnect fun(self: UDPPeer)
---@field GetID fun(self: UDPPeer): number
---@field GetRTT fun(self: UDPPeer): number
---@field IsValid fun(self: UDPPeer): boolean
---@field InvalidID number

---@class UDPHost
---@field Connect fun(self: UDPHost, addr: NetAddress): UDPPeer
---@field Send fun(self: UDPHost, peer: UDPPeer, net_msg: any, channel_id: number, flags: UDPPacketFlags?)
---@field Broadcast fun(self: UDPHost, net_msg: any, channel_id: number, flags: UDPPacketFlags?)
---@field Flush fun(self: UDPHost)
---@field HandleIncomingNetPacket fun(self: UDPHost)
---@field GetPeer fun(self: UDPHost, id: number): UDPPeer

-- Base `EventSystem` methods, equivalent to the C++ bindings in
-- common/src/scripts/script_binding.cpp. The `EventSystem` class is completed
-- in the generated proto_meta.lua fragment (it adds the `AddNetMsg_<X>Event`
-- methods registered by proto_event_binding.cpp), see `CommonContext.GetEventSystem`.
---@class EventSystemBase
---@field AddTimerEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: TimerEvent)): EventListenerID
---@field AddTimerStopEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: TimerStopEvent)): EventListenerID
---@field AddTriggerEnterEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: TriggerEnterEvent)): EventListenerID
---@field AddTriggerLeaveEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: TriggerLeaveEvent)): EventListenerID
---@field AddTriggerTouchEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: TriggerTouchEvent)): EventListenerID
---@field AddDebugEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: DebugEvent)): EventListenerID
---@field AddUIMouseHoverEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UIMouseHoverEvent)): EventListenerID
---@field AddUIMouseDownEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UIMouseDownEvent)): EventListenerID
---@field AddUIMouseUpEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UIMouseUpEvent)): EventListenerID
---@field AddUIMouseClickedEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UIMouseClickedEvent)): EventListenerID
---@field AddUICheckToggledEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UICheckToggledEvent)): EventListenerID
---@field AddUIDragEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: UIDragEvent)): EventListenerID
---@field AddAnimationEndEvent fun(self: EventSystemBase, cb: fun(id: EventListenerID, event: AnimationEndEvent)): EventListenerID
---@field Remove fun(self: EventSystemBase, id: EventListenerID)

---@class EventSystem : EventSystemBase
---@field AddTimerEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: TimerEvent)): EventListenerID
---@field AddTimerStopEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: TimerStopEvent)): EventListenerID
---@field AddTriggerEnterEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: TriggerEnterEvent)): EventListenerID
---@field AddTriggerLeaveEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: TriggerLeaveEvent)): EventListenerID
---@field AddTriggerTouchEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: TriggerTouchEvent)): EventListenerID
---@field AddDebugEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: DebugEvent)): EventListenerID
---@field AddUIMouseHoverEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UIMouseHoverEvent)): EventListenerID
---@field AddUIMouseDownEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UIMouseDownEvent)): EventListenerID
---@field AddUIMouseUpEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UIMouseUpEvent)): EventListenerID
---@field AddUIMouseClickedEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UIMouseClickedEvent)): EventListenerID
---@field AddUICheckToggledEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UICheckToggledEvent)): EventListenerID
---@field AddUIDragEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: UIDragEvent)): EventListenerID
---@field AddAnimationEndEvent fun(self: EventSystem, cb: fun(id: EventListenerID, event: AnimationEndEvent)): EventListenerID
---@field Remove fun(self: EventSystem, id: EventListenerID)

---@class TilemapDetourManager
---@field Load fun(self: TilemapDetourManager, path: Path): TilemapDetourDataHandle
---@field SetCurDetourData fun(self: TilemapDetourManager, handle: TilemapDetourDataHandle)
---@field GetCurDetourData fun(self: TilemapDetourManager): TilemapDetourDataHandle

---@class CommonContext
---@field GetScriptManager fun(self: CommonContext): ScriptComponentManager
---@field GetAssetsManager fun(self: CommonContext): AssetsManager
---@field GetSceneManager fun(self: CommonContext): SceneManager
---@field GetTime fun(self: CommonContext): Time
---@field GetTimerManager fun(self: CommonContext): TimerManager
---@field GetTransformManager fun(self: CommonContext): TransformManager
---@field GetTriggerComponentManager fun(self: CommonContext): TriggerComponentManager
---@field GetRelationshipManager fun(self: CommonContext): RelationshipManager
---@field GetBindPointsComponentManager fun(self: CommonContext): BindPointsComponentManager
---@field GetCCTManager fun(self: CommonContext): CCTManager
---@field GetPhysicsScene fun(self: CommonContext): PhysicsScene
---@field GetStaticCollisionManager fun(self: CommonContext): StaticCollisionManager
---@field GetEventDebugger fun(self: CommonContext): EventDebugger
---@field GetDebugDraw fun(self: CommonContext): DebugDraw
---@field GetTilemapCollisionComponentManager fun(self: CommonContext): TilemapCollisionComponentManager
---@field GetEntityNameManager fun(self: CommonContext): EntityNameManager
---@field GetTilemapDetourManager fun(self: CommonContext): TilemapDetourManager
---@field GetNetHost fun(self: CommonContext): UDPHost?
---@field GetCommonConfig fun(self: CommonContext): CommonConfig
---@field GetEventSystem fun(self: CommonContext): EventSystem
---@field Log fun(self: CommonContext, ...: any)

-- Sub-namespace (enum-like) value tables exposed under TL_Common.
---@class TilemapLayerTypeNamespace
---@field Tiled number
---@field Object number
---@field Image number

---@class TilemapObjectTypeNamespace
---@field None number
---@field Point number
---@field Circle number
---@field Rect number
---@field Polygon number

---@class UDPPacketFlagNamespace
---@field Reliable number
---@field Unsequenced number
---@field UnreliableFragment number

-- The runtime global table created by `beginNamespace("TL_Common")`.
---@class TL_Common
-- entities / ids
---@field null_entity Entity
---@field null_timer_id TimerID
---@field null_event_listener_id EventListenerID
---@field null_trigger_id TriggerID
-- constructors and statics
---@field UUID UUIDCtor
---@field Path PathCtor
---@field Vec2 Vec2Ctor
---@field Vec2UI Vec2UICtor
---@field Color ColorCtor
---@field Degrees DegreesCtor
---@field Radians RadiansCtor
---@field Transform fun(): Transform
---@field Region fun(): Region
---@field CollisionGroup fun(): CollisionGroup
---@field BindPoints fun(): BindPoints
---@field DebugEvent fun(): DebugEvent
---@field NetAddress NetAddressCtor
---@field UDPPeer fun(): UDPPeer
---@field UDPPacketFlags fun(...: number): UDPPacketFlags
-- handle constructors (default-constructed, empty handles)
---@field ImageHandle fun(): ImageHandle
---@field SceneHandle fun(): SceneHandle
---@field PrefabHandle fun(): PrefabHandle
---@field AnimationHandle fun(): AnimationHandle
---@field TilemapHandle fun(): TilemapHandle
---@field FontHandle fun(): FontHandle
-- static class tables (for static member access)
---@field DebugDraw { kOneFrame: TimeType, kAlways: TimeType }
-- sub-namespaces
---@field TilemapLayerType TilemapLayerTypeNamespace
---@field TilemapObjectType TilemapObjectTypeNamespace
---@field UDPPacketFlag UDPPacketFlagNamespace
-- free functions
---@field GetContext fun(): CommonContext
---@field GetAngle fun(from: Vec2, to: Vec2): Radians
---@field DecomposeVector fun(velocity: Vec2, tangent: Vec2): DecompositionResult
---@field Rotate fun(v: Vec2, degrees: Degrees): Vec2
TL_Common = {}
