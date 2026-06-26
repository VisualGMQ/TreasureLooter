local ClientLogicComponent = require("@client/components/logic")
local Component = require("@common/components/component")
local ClientAttackComponent = require("@client/components/attack")

local _M = {}
_M.__index = _M
setmetatable(_M, ClientLogicComponent)

type WeaponLogicData = {}

export type WeaponLogic = typeof(_M) & WeaponLogicData & ClientLogicComponent.ClientLogicComponent

function _M.new(gameobject):WeaponLogic
    local self = Component.new(gameobject) :: WeaponLogic
    return setmetatable(self, _M)
end

function _M.OnUpdate(self:WeaponLogic, elapse_time:TimeType)
    local go = self:GetGameObject()
    if not go.m_attack_component then
        return
    end

    local ctx = TL_Client.GetContext()
    local input_manager = ctx:GetInputManager()

    local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)

    local attack_rot = go.m_attack_component:GetAttackDegrees()
    attack_rot += TL_Common.Degrees(math.sign(axises.x) * 2)
    local dir = TL_Common.Rotate(ClientAttackComponent.k_init_forward_dir, attack_rot)

    go.m_transform.m_rotation = attack_rot

    go.m_attack_component:SetAimDir(dir)

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) then
        if go.m_attack_component then
            go.m_attack_component:Attack()
        end
    end
end

return _M
