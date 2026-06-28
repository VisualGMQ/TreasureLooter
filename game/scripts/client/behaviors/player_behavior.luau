local ClientGameObjectBehavior = require("@client/gameobject_behavior")
local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")
local ClientWorld = require("@client/world")

local k_put_down_distance = 20

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type PlayerBehaviorData = {
}

export type PlayerBehavior = typeof(_M) & PlayerBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity:Entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)::PlayerBehavior
    return self
end

function _M.OnInit(self:PlayerBehavior)
    local world = ClientWorld.GetInst() :: ClientWorld.ClientWorld
    local interact = self.m_gameobject.m_interact_component
    interact:SetHintFX(world.m_player_hint)
end

function _M.OnUpdate(self:PlayerBehavior, elapse_time:TimeType)
    if self.m_gameobject.m_hp_component:IsDead() then
        if self.m_gameobject.m_move_component then
            self.m_gameobject.m_move_component:StopMove()
        end
        return
    end

    local ctx = TL_Client.GetContext()
	local input_manager = ctx:GetInputManager()

    local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
    self.m_gameobject.m_move_component:SetDir(axises)
    self.m_gameobject.m_move_component:Update(elapse_time)

    if self.m_gameobject.m_weapon_component then
        local weapon_script = self.m_gameobject.m_weapon_component:GetScript()
        if weapon_script and weapon_script.m_gameobject then
            local weapon_go = weapon_script.m_gameobject :: ClientGameObject.ClientGameObject
            local dir = ClientAttackComponent.k_init_forwad_dir
            if self.m_gameobject.m_move_component then
                dir = self.m_gameobject.m_move_component:GetLastMoveDirection()
            end
            weapon_go.m_attack_component:SetAimDir(dir)

            local action = input_manager:GetAction("Attack")
            if action:IsPressed(0) and not self.m_gameobject.m_raise_up_component:IsHolding() then
                weapon_go.m_attack_component:Attack(dir)
            end
        end
    end

    if self.m_gameobject.m_interact_component then
        self.m_gameobject.m_interact_component:Update(elapse_time)

        local interact_action = input_manager:GetAction("Interact")
        if interact_action:IsPressed(0) then
            local obj = self.m_gameobject.m_interact_component:GetInteractObject()
            local picked_up = false
            if obj then
                picked_up = self.m_gameobject.m_raise_up_component:TryRaiseUp(obj)
            end
            if not picked_up then
                local facing = self.m_gameobject.m_move_component:GetLastMoveDirection()
                local drop_position = self.m_gameobject.m_transform:GetGlobalPosition() + facing * k_put_down_distance
                self.m_gameobject.m_raise_up_component:PutDown(drop_position)
            end
        end

        local dig_action = input_manager:GetAction("Dig")
        if dig_action:IsPressed(0) then
            local obj = self.m_gameobject.m_interact_component:GetInteractObject()
            if obj then
                local go = obj :: ClientGameObject.ClientGameObject
                if go.m_item_component and go.m_item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Diggable) then
                    if go.m_spawn_object_component then
                        local mine_pos = go.m_transform:GetGlobalPosition()
                        go.m_spawn_object_component:Spawn(mine_pos)
                    end
                end
            end
        end
    end

	ctx:GetCamera():MoveTo(self.m_gameobject.m_transform.m_position)
end


return _M