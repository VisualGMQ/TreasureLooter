local TL = require("@game/tl_types")
local Weapon = require("@game/weapon")
local Common = require("@game/common")
local ScriptBehavior = require("@game/script_behavior")
local ItemBehavior = require("@game/item_behavior")

local Character = {}
Character.__index = Character

---- hurt data

type HurtData = {
	m_hurt_blinking_timer: TL.Timer,
	m_in_red_state: boolean,
}

function Character.createHurtData() : HurtData
    local self = {} :: HurtData
    local ctx: TL.GameContext = TL.GetContext()
	self.m_hurt_blinking_timer = ctx:GetTimerManager():Create(0.1, TL.TimerEventType.Unknown, 8)
	self.m_in_red_state = false
    return self
end

---- pickup data

local PickupData = {}
PickupData.__index = PickupData

export type PickupData = {
    m_items: { ItemBehavior.ItemBehavior },
}

function PickupData.HasObject(self: PickupData)
    return #self.m_items > 0
end

function Character.createPickupData() : PickupData
    local self = {} :: PickupData
    self.m_items = {}
    return setmetatable(self, PickupData)
end

---- character data

type CharacterData = {
    m_entity: TL.Entity,
    m_weapon : Weapon.Weapon?,

    -- from definition
    m_hp: number,
    m_move_left_anim : TL.AnimationHandle,
    m_move_right_anim : TL.AnimationHandle,
    m_move_up_anim : TL.AnimationHandle,
    m_move_down_anim : TL.AnimationHandle,
    m_move_speed : number,
    m_interactable_detect_trigger : TL.Trigger?,
	m_sprite: TL.Sprite,
	m_cct : TL.CharacterController?,
	m_transform : TL.Transform,
	m_anim_player : TL.AnimationPlayer,

    -- from runtime
    m_team_id: TL.TeamID,
	m_direction : Common.Direction,
    m_forward_dir : TL.Vec2,

    m_hurt_data: HurtData,
    m_pickup_data: PickupData,
}

export type Character = typeof(setmetatable({} :: CharacterData, Character))

function Character.new(entity: TL.Entity, definition: TL.CharacterDefinitionHandle, weapon: Weapon.Weapon?): Character
    local self = {} :: CharacterData
    self.m_entity = entity
    self.m_weapon = weapon

    self.m_hp = definition.m_hp
    self.m_move_left_anim = definition.m_move_left_animation
    self.m_move_right_anim = definition.m_move_right_animation
    self.m_move_up_anim = definition.m_move_up_animation
    self.m_move_down_anim = definition.m_move_down_animation
    self.m_move_speed = definition.m_speed
    self.m_team_id = TL.TeamID.Team_invalid
    self.m_forward_dir = TL.Vec2.Y_UNIT

    local ctx: TL.GameContext = TL.GetContext()
    local trigger_manager = ctx:GetTriggerComponentManager()
    self.m_interactable_detect_trigger = trigger_manager:Get(entity)

	self.m_sprite = ctx:GetSpriteManager():Get(entity)
	self.m_cct = ctx:GetCCTManager():Get(entity)
	self.m_transform = ctx:GetTransformManager():Get(entity)
	self.m_anim_player = ctx:GetAnimationPlayerManager():Get(entity)

    self.m_hurt_data = Character.createHurtData()
    self.m_pickup_data = Character.createPickupData()

    self.m_direction = Common.Direction.Down

    return setmetatable(self, Character)
end

function Character.SubscribeEvents(self:Character,  behavior: ScriptBehavior.ScriptBehavior)
	behavior:SubscribeDebugEvent()
	behavior:SubscribeTriggerEnterEvent()
	behavior:SubscribeTimerEvent()
	behavior:SubscribeTimerStopEvent()
end

function Character.SetTeamID(self: Character, team_id: TL.TeamID)
    self.m_team_id = team_id
end

function Character.GetTeamID(self: Character): TL.TeamID
    return self.m_team_id
end

function Character.GetWeaponEntity(self: Character): TL.Entity
    return self._weapon_entity
end

function Character.GetWeaponControlNode(self: Character): TL.Entity
    return self._weapon_control_node
end

function Character.Attack(self:Character)
    if self.m_weapon then
        self.m_weapon:Attack()
    end
end

function Character.Move(self:Character, dir: TL.Vec2, duration: TL.TimeType)
    if dir == TL.Vec2.ZERO then
		if self.m_anim_player ~= nil then
			self.m_anim_player:Stop()
		end
        return
	elseif self.m_weapon and not self.m_weapon.m_anim_player:IsPlaying() then
		self.m_weapon:ChangeDir(dir:Normalize())
	end

    self.m_forward_dir = dir

	local old_dir = self.m_direction
	if dir.x < 0 then
		self.m_direction = Common.Direction.Left
	end
	if dir.x > 0 then
		self.m_direction = Common.Direction.Right
	end
	if dir.y < 0 then
		self.m_direction = Common.Direction.Up
	end
	if dir.y > 0 then
		self.m_direction = Common.Direction.Down
	end

	if self.m_cct ~= nil then
		local disp = dir * self.m_move_speed * duration
		self.m_cct:MoveAndSlide(disp)
		self.m_transform.m_position = self.m_cct:GetPosition()
	end

	if (not self.m_anim_player:IsPlaying() and dir ~= TL.Vec2.ZERO) or old_dir ~= self.m_direction then
		if self.m_direction == Common.Direction.Up then
			self.m_anim_player:ChangeAnimation(self.m_move_up_anim)
		elseif self.m_direction == Common.Direction.Left then
			self.m_anim_player:ChangeAnimation(self.m_move_left_anim)
		elseif self.m_direction == Common.Direction.Right then
			self.m_anim_player:ChangeAnimation(self.m_move_right_anim)
		elseif self.m_direction == Common.Direction.Down then
			self.m_anim_player:ChangeAnimation(self.m_move_down_anim)
		end
		self.m_anim_player:Play()
	end

	if dir == TL.Vec2.ZERO then
		if self.m_direction == Common.Direction.Up then
			self.m_sprite.m_region.m_topleft = TL.Vec2(16, 0)
		elseif self.m_direction == Common.Direction.Left then
			self.m_sprite.m_region.m_topleft = TL.Vec2(32, 0)
		elseif self.m_direction == Common.Direction.Right then
			self.m_sprite.m_region.m_topleft = TL.Vec2(48, 0)
		elseif self.m_direction == Common.Direction.Down then
			self.m_sprite.m_region.m_topleft = TL.Vec2(0, 0)
		end
	end

end

function Character.Hurt(self: Character, entity: TL.Entity, damage: number)
	if not self.m_hurt_data.m_hurt_blinking_timer:IsRunning() then
		self.m_hp = self.m_hp - damage
		self.m_hurt_data.m_hurt_blinking_timer:Start()
	end

	if self.m_hp <= 0 then
		local ctx: TL.GameContext = TL.GetContext()
		local scene = ctx:GetSceneManager():GetCurrentScene()
		scene:RemoveEntity(entity)
	end
end

--- @return pickup success
function Character.TryPickup(self: Character): boolean
    local interactable_object = self:GetNearbyInteractableObject()
    if not interactable_object then
        return false
    end

    local entity = interactable_object:GetOwner()
    local ctx: TL.GameContext = TL.GetContext()
    local script = ctx:GetScriptManager():Get(entity)

    if not script or not script.m_item then
        TL.Log("object is not item, can't pickup")
        return false
    end

    script = script :: ItemBehavior.ItemBehavior
    if not script.m_item.m_behavior:Has(TL.ItemBehavior.Pickupable) then
        return false
    end

    if not self.m_pickup_data:HasObject() then
        table.insert(self.m_pickup_data.m_items, script)
        script:PickupTo(self.m_entity, #self.m_pickup_data.m_items - 1)
        return true
    end

    local pickup_item_entity = script:GetEntity()

    --[[ handle stackable
    if script.m_item.m_behavior:Has(TL.ItemBehavior.Stackable) and
        self.m_pickup_data:HasObject() and
        self.m_pickup_data.m_items[1]:GetEntity() == pickup_item_entity then
        table.insert(self.m_pickup_data.m_items, script)
        script:PickupTo(self.m_entity)
        return true
    else
        return false
    end
    --]]
    return false
end

function Character.PutItemDown(self: Character)
    if not self.m_pickup_data:HasObject() then
        return
    end

    local item = nil

    local back_index = #self.m_pickup_data.m_items
    item = self.m_pickup_data.m_items[back_index]
    table.remove(self.m_pickup_data.m_items, back_index)

    local position = self.m_transform:GetGlobalPosition()
    position += self:GetForwardDir() * 20
    item:Putdown(position)
end

function Character.GetForwardDir(self: Character): TL.Vec2
    return self.m_forward_dir
end

function Character.OnTimerEvent(self: Character, event: TL.TimerEvent)
	local timer_id = event:GetID()

	if timer_id == self.m_hurt_data.m_hurt_blinking_timer:GetID() then
		if self.m_hurt_data.m_in_red_state then
			self.m_sprite.m_color = TL.Color.White
		else
			self.m_sprite.m_color = TL.Color.Red
		end
		self.m_hurt_data.m_in_red_state = not self.m_hurt_data.m_in_red_state
	end
end

function Character.OnTimerStopEvent(self: Character, event: TL.TimerStopEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_hurt_data.m_hurt_blinking_timer:GetID() then
		self.m_sprite.m_color = TL.Color.White
		self.m_hurt_data.m_in_red_state = false
	end
end

function Character.GetNearbyInteractableObject(self: Character): TL.PhysicsShape?
    if not self.m_interactable_detect_trigger then
        return nil
    end

    local shapes = self.m_interactable_detect_trigger:GetTouchingShapes()
    if #shapes == 0 then
        return nil
    end

    local nearest_shape = shapes[1]
    local nearest_dist = math.huge
    for _, shape: TL.PhysicsShape in ipairs(shapes) do
        local position = shape:GetPosition()
        local dist = (self.m_transform:GetGlobalPosition() - position):Length()
        if dist < nearest_dist then
            nearest_dist = dist
            nearest_shape = shape
        end
    end

    return nearest_shape
end

function Character.IsHoldingObject(self: Character)
    return self.m_pickup_data:HasObject()
end

return Character
