local ClientGameObjectBehavior = require("@client/gameobject_behavior")
local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type PlayerBehaviorData = {}

export type PlayerBehavior = typeof(_M) & PlayerBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity:Entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)::PlayerBehavior
    return self
end

function _M.OnInit(self:PlayerBehavior)
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
            if action:IsPressed(0) then
                weapon_go.m_attack_component:Attack(dir)
            end
        end
    end

	ctx:GetCamera():MoveTo(self.m_gameobject.m_transform.m_position)
end


return _M