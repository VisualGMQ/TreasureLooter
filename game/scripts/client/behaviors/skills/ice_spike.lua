local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class IceSpikeBehaviorData

---@class IceSpikeBehavior : ClientGameObjectBehavior
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity LogicEntity
---@return IceSpikeBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self IceSpikeBehavior
    return self
end

---@param caster_entity LogicEntity
function _M:Start(caster_entity)
    local go = self.m_gameobject
    if go.m_skill_components and #go.m_skill_components > 0 then
        local skill = go.m_skill_components[1]
        skill:Cast(caster_entity)
        skill:SetHitCallback(function(caster, target)
        end)
    end
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)

    local go = self.m_gameobject
    if go and go.m_skill_components and #go.m_skill_components > 0 then
        local skill = go.m_skill_components[1]
        if not skill._is_casting then
            local ctx = TL_Client.GetContext()
            local scene = ctx:GetSceneManager():GetCurrentScene()
            scene:RemoveEntity(self:GetEntity())
        end
    end
end

return _M
