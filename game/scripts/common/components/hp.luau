local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Component})


type HpComponentData = {
    m_hp: number,
}
export type HpComponent = typeof(_M) & HpComponentData & Component.Component

function _M.new(gameobject, hp:number):HpComponent
    local self = Component.new(gameobject) :: HpComponent
    self.m_hp = hp
    return setmetatable(self, _M)
end

function _M.Hurt(self:HpComponent, damage: number)
    self.m_hp -= damage
end

function _M.IsDead(self:HpComponent):boolean
    return self.m_hp <= 0
end

function _M.IsAlive(self:HpComponent):boolean
    return not self:IsDead()
end


return _M