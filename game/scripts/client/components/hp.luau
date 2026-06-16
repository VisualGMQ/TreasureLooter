local HpComponent = require("@common/components/hp")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = HpComponent})

type ClientHpComponentData = {
    _anim_player:AnimationPlayer,
    _hurt_anim:AnimationHandle,
    _dead_anim:AnimationHandle,
}

export type ClientHpComponent = typeof(_M) & ClientHpComponentData & HpComponent.HpComponent

export type ClientHpComponentDefinition = {
    m_hp:number,
    m_anim_player:AnimationPlayer,
    m_hurt_anim:AnimationHandle,
    m_dead_anim:AnimationHandle,
}

function _M.new(gameobject, definition:ClientHpComponentDefinition):ClientHpComponent
    local self = HpComponent.new(gameobject, definition.m_hp) :: ClientHpComponent
    self._anim_player = definition.m_anim_player
    self._hurt_anim = definition.m_hurt_anim
    self._dead_anim = definition.m_dead_anim
    return setmetatable(self, _M)
end

function _M.Hurt(self:ClientHpComponent, damage:number)
    HpComponent.Hurt(self, damage)

    if self:IsDead() then
        self._anim_player:ChangeAnimation(self._dead_anim)
    else
        self._anim_player:ChangeAnimation(self._hurt_anim)
    end
    self._anim_player:SetLoop(0)
    self._anim_player:Play()
end


return _M

