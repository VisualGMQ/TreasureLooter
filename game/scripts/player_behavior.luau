local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")
local Character = require("@game/character")

local PlayerBehavior = {}
PlayerBehavior.__index = PlayerBehavior
setmetatable(PlayerBehavior, { __index = ScriptBehavior })

type PlayerBehaviorData = {
    m_character: Character.Character,

    m_interactable_hint_entity: TL.Entity,
}

export type PlayerBehavior = typeof(setmetatable({}::PlayerBehaviorData, PlayerBehavior)) & ScriptBehavior.ScriptBehavior

function PlayerBehavior.new(entity: TL.Entity): PlayerBehavior
	local self = ScriptBehavior.new(entity)
	return setmetatable(self, PlayerBehavior)
end

function PlayerBehavior._init(self: PlayerBehavior, character: Character.Character, player_definition: TL.PlayerRelatedDefinition)
    self.m_character = character
    self.m_interactable_hint_entity = TL.null_entity

    local ctx: TL.GameContext = TL.GetContext()
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local handle = prefab_manager:Create()
    handle.m_transform = TL.Transform()
    handle.m_sprite.m_image = player_definition.m_interactable_hint_image
    handle.m_sprite.m_size = player_definition.m_interactable_hint_image:GetSize()
    handle.m_sprite.m_region.m_size = player_definition.m_interactable_hint_image:GetSize()
    handle.m_draw_order = TL.DrawOrderDefinition()
    handle.m_draw_order.m_draw_layer = TL.DrawLayer.TilemapArch
    handle.m_animation = TL.AnimationPlayerDefinition()
    handle.m_animation.m_auto_play = true
    handle.m_animation.m_animation = player_definition.m_interactable_hint_animation;
    handle.m_animation.m_loop = -1
    handle.m_animation.m_rate = 1.0
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL.Log("scene not exists")
        return
    end
    self.m_interactable_hint_entity = scene:Instantiate(handle)
    prefab_manager:Unload(handle)

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    root_relationship:AddChild(self.m_interactable_hint_entity)

    ctx:GetSpriteManager():Disable(self.m_interactable_hint_entity)
end

function PlayerBehavior.OnQuit(self: PlayerBehavior)
    local ctx: TL.GameContext = TL.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        scene:RemoveEntity(self.m_interactable_hint_entity)
    end
end

function PlayerBehavior.OnInit(self: PlayerBehavior)
    self.m_character:SubscribeEvents(self)
end

function PlayerBehavior.OnUpdate(self: PlayerBehavior, elapse_time: TL.TimeType)
    local ctx = TL.GetContext()
	local input_manager = ctx:GetInputManager()
	local action = input_manager:GetAction("Attack")
	local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
	if (action:IsPressed(0)) then
		self.m_character:Attack()
	end
	self.m_character:Move(axises, elapse_time)
	ctx:GetCamera():MoveTo(self.m_character.m_transform.m_position)
end

function PlayerBehavior.OnRender(self: PlayerBehaviorData)
    self:ShowDetectedInteractableItems(self.m_character.m_transform:GetGlobalPosition())
end

function PlayerBehavior.ShowDetectedInteractableItems(self: PlayerBehavior, entity_position: TL.Vec2)
    if not self.m_character.m_interactable_detect_trigger then
        return
    end

    local shapes = self.m_character.m_interactable_detect_trigger:GetTouchingShapes()
    local ctx: TL.GameContext = TL.GetContext()
    if #shapes == 0 then
        ctx:GetSpriteManager():Disable(self.m_interactable_hint_entity)
        local anim_player = ctx:GetAnimationPlayerManager():Get(self.m_interactable_hint_entity)
        anim_player:Stop()
        return
    end

    local nearest_shape = shapes[1]
    local nearest_dist = math.huge
    for _, shape: TL.PhysicsShape in ipairs(shapes) do
        local position = shape:GetPosition()
        local dist = (entity_position - position):Length()
        if dist < nearest_dist then
            nearest_dist = dist
            nearest_shape = shape
        end
    end

    ctx:GetSpriteManager():Enable(self.m_interactable_hint_entity)
    local transform = ctx:GetTransformManager():Get(self.m_interactable_hint_entity)
    transform.m_position = nearest_shape:GetPosition()
    transform.m_scale.x = 0.5
    transform.m_scale.y = 0.5
    transform.m_position.y -= 10
    local anim_player = ctx:GetAnimationPlayerManager():Get(self.m_interactable_hint_entity)
    anim_player:Play()
end

function PlayerBehavior.OnDebugEvent(self: PlayerBehavior, event: TL.DebugEvent)
	TL.Log("debug event here")
end

function PlayerBehavior.OnTriggerEnterEvent(self: PlayerBehavior, event: TL.TriggerEnterEvent)
	local dst_entity = event:GetOverlapResult().m_dst_entity
	if dst_entity == TL.null_entity or dst_entity == self:GetEntity() then
		return
	end

	local ctx: TL.GameContext = TL.GetContext()
	local script_mgr = ctx:GetScriptManager()
	local dst_script = script_mgr:Get(dst_entity)

    -- check is character
    if dst_script and dst_script.m_character then
        dst_script.m_character:Hurt(dst_script:GetEntity(), 1)
    end
end

return PlayerBehavior
