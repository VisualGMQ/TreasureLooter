local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")
local TL_Schema = require("@type_hints/tl_schema_types")


local Weapon = require("@client/weapon")
local Common = require("@client/common")
local ScriptBehavior = require("@client/script_behavior")
local ItemBehavior = require("@client/item_behavior")

local Character = {}
Character.__index = Character

---- hurt data

type HurtData = {
	m_hurt_blinking_timer: TL_Common.Timer,
	m_in_red_state: boolean,
}

function Character.createHurtData() : HurtData
    local self = {} :: HurtData
    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
	self.m_hurt_blinking_timer = ctx:GetTimerManager():Create(0.1, TL_Schema.TimerEventType.Unknown, 8)
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

-- digging data

local DiggingData = {}
DiggingData.__index = DiggingData

export type DiggingData = {
	m_target : ItemBehavior.ItemBehavior?,
	m_digging_countdown_timer : TL_Common.Timer,
}

function DiggingData.Start(self: DiggingData, item: ItemBehavior.ItemBehavior): boolean
	if self.m_target ~= nil and self.m_target ~= item then	
		self:Stop()
	end
	
	self.m_target = item
	self.m_digging_countdown_timer:SetInterval(self.m_target.m_item.m_dig_time)
	self.m_digging_countdown_timer:Start()	
	TL_Common.Log("start digging")
	return true
end

function DiggingData.Stop(self: DiggingData)
	self.m_digging_countdown_timer:Stop()
	self.m_target = nil
	TL_Common.Log("stop digging")
end

function DiggingData.OnTimerEvent(self: DiggingData, event: TL_Common.TimerEvent)
	local type = event:GetEventType()
	if type ~= TL_Schema.TimerEventType.DigFinish then
		return
	end
	-- TODO: Create mineral fragment
	
	TL_Common.Log("generate mineral fragment")
end

function DiggingData.GetTarget(self: DiggingData): ItemBehavior.ItemBehavior?
	return self.m_target
end

function DiggingData.IsDigging(self: DiggingData): boolean
	return self.m_target ~= nil
end

function DiggingData.OnQuit(self: DiggingData)
	local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
	ctx:GetTimerManager():Remove(self.m_digging_countdown_timer:GetID())
end

function Character.createDiggingData(): DiggingData
	local self = {} :: DiggingData
	local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
	self.m_digging_countdown_timer = ctx:GetTimerManager():Create(0, TL_Schema.TimerEventType.DigFinish, -1)
	return setmetatable(self, DiggingData)
end

---- character data

type CharacterData = {
    m_entity: TL_Common.Entity,
    m_weapon : Weapon.Weapon?,
    m_pickaxe : Weapon.Weapon?,
	m_digging_data : DiggingData,

    -- from definition
    m_hp: number,
    m_move_left_anim : TL_Schema.AnimationHandle,
    m_move_right_anim : TL_Schema.AnimationHandle,
    m_move_up_anim : TL_Schema.AnimationHandle,
    m_move_down_anim : TL_Schema.AnimationHandle,
    m_move_speed : number,
	m_sprite: TL_Client.Sprite,
	m_cct : TL_Common.CharacterController?,
	m_transform : TL_Common.Transform,
	m_anim_player : TL_Client.AnimationPlayer,

    -- from runtime
    m_team_id: TL_Schema.TeamID,
	m_direction : Common.Direction,
    m_forward_dir : TL_Common.Vec2,

    m_hurt_data: HurtData,
    m_pickup_data: PickupData,
}

export type Character = typeof(setmetatable({} :: CharacterData, Character))

function Character.new(entity: TL_Common.Entity, definition: TL_Schema.CharacterDefinitionHandle, weapon: Weapon.Weapon?, pickexe: Weapon.Weapon?): Character
    local self = {} :: CharacterData
    self.m_entity = entity
    self.m_weapon = weapon
    self.m_pickaxe = pickexe
	self.m_digging_data = Character.createDiggingData()

    self.m_hp = definition.m_hp
    self.m_move_left_anim = definition.m_move_left_animation
    self.m_move_right_anim = definition.m_move_right_animation
    self.m_move_up_anim = definition.m_move_up_animation
    self.m_move_down_anim = definition.m_move_down_animation
    self.m_move_speed = definition.m_speed
    self.m_team_id = TL_Schema.TeamID.invalid
    self.m_forward_dir = TL_Common.Vec2.Y_UNIT

    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

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

function Character.SetTeamID(self: Character, team_id: TL_Schema.TeamID)
    self.m_team_id = team_id
end

function Character.GetTeamID(self: Character): TL_Schema.TeamID
    return self.m_team_id
end

function Character.GetWeaponEntity(self: Character): TL_Common.Entity
    return self._weapon_entity
end

function Character.GetWeaponControlNode(self: Character): TL_Common.Entity
    return self._weapon_control_node
end

function Character.Attack(self:Character)
    if self.m_weapon then
        self.m_weapon:Attack()
    end
end

function Character.Move(self:Character, dir: TL_Common.Vec2, duration: TL_Common.TimeType)
    if dir == TL_Common.Vec2.ZERO then
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

	if (not self.m_anim_player:IsPlaying() and dir ~= TL_Common.Vec2.ZERO) or old_dir ~= self.m_direction then
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

	if dir == TL_Common.Vec2.ZERO then
		if self.m_direction == Common.Direction.Up then
			self.m_sprite.m_region.m_topleft = TL_Common.Vec2(16, 0)
		elseif self.m_direction == Common.Direction.Left then
			self.m_sprite.m_region.m_topleft = TL_Common.Vec2(32, 0)
		elseif self.m_direction == Common.Direction.Right then
			self.m_sprite.m_region.m_topleft = TL_Common.Vec2(48, 0)
		elseif self.m_direction == Common.Direction.Down then
			self.m_sprite.m_region.m_topleft = TL_Common.Vec2(0, 0)
		end
	end

end

function Character.Hurt(self: Character, damage: number)
	if not self.m_hurt_data.m_hurt_blinking_timer:IsRunning() then
		self.m_hp = self.m_hp - damage
		self.m_hurt_data.m_hurt_blinking_timer:Start()
	end

	if self.m_hp <= 0 then
		local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
		local scene = ctx:GetSceneManager():GetCurrentScene()
		scene:RemoveEntity(self.m_entity)
	end
end

--- @return pickup success
function Character.TryPickup(self: Character, script: ItemBehavior.ItemBehavior): boolean
    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    if not self.m_pickup_data:HasObject() then
        table.insert(self.m_pickup_data.m_items, script)
        script:PickupTo(self.m_entity, #self.m_pickup_data.m_items - 1)
        return true
    end

    if not script.m_item:IsSameType(self.m_pickup_data.m_items[1].m_item) then
        return false
    end

    table.insert(self.m_pickup_data.m_items, script)
    script:PickupTo(self.m_entity, #self.m_pickup_data.m_items)
    return true
end

function Character.StartDig(self: Character, item: ItemBehavior.ItemBehavior): boolean
	self.m_digging_data:Start(item)
end

function Character.StopDig(self: Character, script: ItemBehavior.ItemBehavior)
	self.m_digging_data:Stop()
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

function Character.GetForwardDir(self: Character): TL_Common.Vec2
    return self.m_forward_dir
end

function Character.IsHoldingObject(self: Character)
    return self.m_pickup_data:HasObject()
end

function Character.OnQuit(self: Character)
    if self.m_weapon then
        self.m_weapon:OnQuit()
    end

    if self.m_pickaxe then
        self.m_pickaxe:OnQuit()
    end
	
	self.m_digging_data:OnQuit()
end

function Character.OnTriggerEnterEvent(self: Character, event: TL_Common.TriggerEnterEvent)
    if self.m_weapon then
        self.m_weapon:OnTriggerEnterEvent(event)
    end

    if self.m_pickaxe then
        self.m_pickaxe:OnTriggerEnterEvent(event)
    end
end

function Character.OnTimerEvent(self: Character, event: TL_Common.TimerEvent)
	local timer_id = event:GetID()

	if timer_id == self.m_hurt_data.m_hurt_blinking_timer:GetID() then
		if self.m_hurt_data.m_in_red_state then
			self.m_sprite.m_color = TL_Common.Color.White
		else
			self.m_sprite.m_color = TL_Common.Color.Red
		end
		self.m_hurt_data.m_in_red_state = not self.m_hurt_data.m_in_red_state
	end
	
	self.m_digging_data:OnTimerEvent(event)
end

function Character.OnTimerStopEvent(self: Character, event: TL_Common.TimerStopEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_hurt_data.m_hurt_blinking_timer:GetID() then
		self.m_sprite.m_color = TL_Common.Color.White
		self.m_hurt_data.m_in_red_state = false
	end
end

return Character


