local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class FireballBehaviorData
---@field m_fire_dir Vec2
---@field m_fire_speed number

---@class FireballBehavior : ClientGameObjectBehavior
---@field m_fire_dir Vec2
---@field m_fire_speed number
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity Entity
---@return FireballBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self FireballBehavior
    self.m_fire_dir = TL_Common.Vec2(1, 0)
    self.m_fire_speed = 100
    return self
end

---@param dir Vec2
function _M:SetFireDir(dir)
    self.m_fire_dir = dir
end

---@param speed number
function _M:SetFireSpeed(speed)
    self.m_fire_speed = speed
end

---@param caster_entity Entity
function _M:Start(caster_entity)
    local go = self.m_gameobject
    if go.m_skill_components and #go.m_skill_components > 0 then
        local skill = go.m_skill_components[1]
        skill:Cast(caster_entity)
        skill:SetHitCallback(function(caster, target)
            local ctx = TL_Client.GetContext()
            local scene = ctx:GetSceneManager():GetCurrentScene()
            scene:RemoveEntity(self:GetEntity())
        end)
    end
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)

    local go = self.m_gameobject
    if go then
        go.m_transform.m_position = go.m_transform.m_position + self.m_fire_dir * self.m_fire_speed * elapse_time
        go.m_transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(TL_Common.Vec2(0, -1), self.m_fire_dir))
    end
end

return _M
