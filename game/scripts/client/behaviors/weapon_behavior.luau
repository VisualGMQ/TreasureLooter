local ClientGameObjectBehavior = require("@client/gameobject_behavior")
local ClientAttackComponent = require("@client/components/attack")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type WeaponBehaviorData = {}

export type WeaponBehavior = typeof(_M) & WeaponBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity:Entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)::WeaponBehavior
    return self
end

function _M.OnUpdate(self:WeaponBehavior, elapse_time:TimeType)
    if not self.m_gameobject.m_attack_component then
        return
    end

    local ctx = TL_Client.GetContext()
	local input_manager = ctx:GetInputManager()

    local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)

    local attack_rot = self.m_gameobject.m_attack_component:GetAttackDegrees()
    attack_rot += TL_Common.Degrees(math.sign(axises.x) * 2)
    local dir = TL_Common.Rotate(ClientAttackComponent.k_init_forward_dir, attack_rot)

    self.m_gameobject.m_transform.m_rotation = attack_rot

    self.m_gameobject.m_attack_component:SetAimDir(dir)

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) then
        if self.m_gameobject.m_attack_component then
            self.m_gameobject.m_attack_component:Attack()
        end
    end
end

function _M.GetFacingDirection(self:WeaponBehavior): Vec2
    local radians = self.m_gameobject.m_transform.m_rotation:Value()
    return TL_Common.Vec2(math.cos(radians), math.sin(radians))
end


return _M