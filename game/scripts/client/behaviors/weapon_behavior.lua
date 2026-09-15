local ClientGameObjectBehavior = require("client.gameobject_behavior")
local ClientAttackComponent = require("client.components.attack")

---@param x number
---@return integer
local function sign(x)
    if x > 0 then return 1 end
    if x < 0 then return -1 end
    return 0
end

---@class WeaponLogicData

---@class WeaponLogic : ClientGameObjectBehavior
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param gameobject any
---@return WeaponLogic
function _M.new(gameobject)
    local self = ClientGameObjectBehavior.new(gameobject)
    ---@cast self WeaponLogic
    return setmetatable(self, _M)
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    local go = self:GetGameObject()
    if not go.m_attack_component then
        return
    end

    local ctx = TL_Client.GetContext()
    local input_manager = ctx:GetInputManager()

    local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)

    local attack_rot = go.m_attack_component:GetAttackDegrees()
    attack_rot = attack_rot + TL_Common.Degrees(sign(axises.x) * 2)
    local dir = TL_Common.Rotate(ClientAttackComponent.k_init_forward_dir, attack_rot)

    go.m_transform.m_rotation = attack_rot

    go.m_attack_component:SetAimDir(dir)

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) then
        if go.m_attack_component then
            go.m_attack_component:Attack()
        end
    end

    ClientGameObjectBehavior.OnUpdate(self, elapse_time)
end

return _M
