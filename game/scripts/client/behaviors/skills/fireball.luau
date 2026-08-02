local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type FireballBehaviorData = {
    m_fire_dir: Vec2,
    m_fire_speed: number,
}

export type FireballBehavior = typeof(_M) & FireballBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity: Entity): FireballBehavior
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M) :: FireballBehavior
    self.m_fire_dir = TL_Common.Vec2(1, 0)
    self.m_fire_speed = 100
    return self
end

function _M.SetFireDir(self: FireballBehavior, dir: Vec2)
    self.m_fire_dir = dir
end

function _M.SetFireSpeed(self: FireballBehavior, speed: number)
    self.m_fire_speed = speed
end

function _M.Start(self: FireballBehavior, caster_entity: Entity)
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

function _M.OnUpdate(self: FireballBehavior, elapse_time: TimeType)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)

    local go = self.m_gameobject
    if go then
        go.m_transform.m_position = go.m_transform.m_position + self.m_fire_dir * self.m_fire_speed * elapse_time
        go.m_transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(TL_Common.Vec2(0, -1), self.m_fire_dir))
    end
end

return _M
