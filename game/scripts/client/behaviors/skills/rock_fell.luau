local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type RockFellBehaviorData = {}

export type RockFellBehavior = typeof(_M) & RockFellBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity: Entity): RockFellBehavior
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M) :: RockFellBehavior
    return self
end

function _M.Start(self: RockFellBehavior, caster_entity: Entity)
    local go = self.m_gameobject
    if go.m_skill_components and #go.m_skill_components > 0 then
        local skill = go.m_skill_components[1]
        skill:Cast(caster_entity)
        skill:SetHitCallback(function(caster, target)
        end)
    end
end

function _M.OnUpdate(self: RockFellBehavior, elapse_time: TimeType)
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
