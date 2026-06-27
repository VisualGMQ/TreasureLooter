local RaiseUpComponent = require("@common/components/raise_up")

local _M = {
    k_head_lift = 10,
    k_stack_height = 12,
}
_M.__index = _M
setmetatable(_M, RaiseUpComponent)

type ClientRaiseUpComponentData = {
    _origin_anchors:{[Entity]:Vec2},
}

export type ClientRaiseUpComponent = typeof(_M) & ClientRaiseUpComponentData & RaiseUpComponent.RaiseUpComponent

function _M.new(gameobject):ClientRaiseUpComponent
    local self = RaiseUpComponent.new(gameobject) :: ClientRaiseUpComponent
    self._origin_anchors = {}
    return setmetatable(self, _M)
end

-- lift the carried item's sprite above the head via its anchor (anchor.y down(+) -> sprite up).
-- the entity transform stays on the owner, so y-sorting is unaffected.
function _M.onItemRaise(self:ClientRaiseUpComponent, gameobject, index:number)
    local entity = gameobject:GetEntity()
    local sprite = TL_Client.GetContext():GetSpriteManager():Get(entity)
    if not sprite then
        return
    end
    local anchor_x = sprite.m_anchor.x
    local anchor_y = sprite.m_anchor.y
    self._origin_anchors[entity] = TL_Common.Vec2(anchor_x, anchor_y)
    sprite.m_anchor = TL_Common.Vec2(anchor_x, anchor_y + _M.k_head_lift + _M.k_stack_height * index)
end

function _M.onItemPutDown(self:ClientRaiseUpComponent, gameobject)
    local entity = gameobject:GetEntity()
    local origin = self._origin_anchors[entity]
    if not origin then
        return
    end
    local sprite = TL_Client.GetContext():GetSpriteManager():Get(entity)
    if sprite then
        sprite.m_anchor = TL_Common.Vec2(origin.x, origin.y)
    end
    self._origin_anchors[entity] = nil
end

return _M
