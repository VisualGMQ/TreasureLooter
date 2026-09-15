local RaiseUpComponent = require("common.components.raise_up")

---@class ClientRaiseUpComponent : RaiseUpComponent
---@field _origin_anchors table<Entity, Vec2>
local _M = {
    k_head_lift = 10,
    k_stack_height = 12,
}
_M.__index = _M
setmetatable(_M, { __index = RaiseUpComponent })

---@param gameobject any
---@return ClientRaiseUpComponent
function _M.new(gameobject)
    local self = RaiseUpComponent.new(gameobject)
    ---@cast self ClientRaiseUpComponent
    self._origin_anchors = {}
    return setmetatable(self, _M)
end

---@param gameobject any
---@return boolean
function _M:TryRaiseUp(gameobject)
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

---@param position Vec2
function _M:PutDown(position)
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
