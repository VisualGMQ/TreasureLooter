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

function _M.TryRaiseUp(self:ClientRaiseUpComponent, gameobject):boolean
    -- Stop spawner first, before common TryRaiseUp reparents the item
    if gameobject.m_spawned_component then
        gameobject.m_spawned_component:Stop()
    end

    local last_item_index = self:GetHoldingItemCount()
    if RaiseUpComponent.TryRaiseUp(self, gameobject) then
        local entity = gameobject:GetEntity()
        local sprite = TL_Client.GetContext():GetSpriteManager():Get(entity)
        if not sprite then
            return true
        end
        local anchor_x = sprite.m_anchor.x
        local anchor_y = sprite.m_anchor.y
        self._origin_anchors[entity] = TL_Common.Vec2(anchor_x, anchor_y)
        sprite.m_anchor = TL_Common.Vec2(anchor_x, anchor_y + _M.k_head_lift + _M.k_stack_height * last_item_index)
    end
    return true
end

function _M.PutDown(self:ClientRaiseUpComponent, position:Vec2)
    local putdown_item = RaiseUpComponent.PutDown(self, position)
    if putdown_item then
        local entity = putdown_item:GetEntity()
        local origin = self._origin_anchors[entity]
        if not origin then
            return putdown_item
        end
        local sprite = TL_Client.GetContext():GetSpriteManager():Get(entity)
        if sprite then
            sprite.m_anchor = TL_Common.Vec2(origin.x, origin.y)
        end
        self._origin_anchors[entity] = nil
    end
    return putdown_item
end

return _M
