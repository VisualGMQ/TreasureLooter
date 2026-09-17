local MoveComponent = require("common.components.move")

---@class ServerMoveComponentDefinition
---@field m_cct CharacterController|nil
--- The speed of the character definition is unused now: the component has its
--- own m_max_speed (see MoveComponent).
---@field m_speed number

---@class ServerMoveComponent : MoveComponent
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

---@param gameobject any
---@param definition ServerMoveComponentDefinition
---@return ServerMoveComponent
function _M.new(gameobject, definition)
    local self = MoveComponent.new(gameobject, definition.m_cct)
    ---@cast self ServerMoveComponent
    return setmetatable(self, _M)
end

-- The input displacement comes from the client, but the velocity given by a
-- collision is the server's own, so it is applied on top of it: that is what
-- pushes the other character away for every client.
---@param elapse_time TimeType?
function _M:Update(elapse_time)
    if elapse_time then
        self:AddMoveDisp(self.m_velocity * elapse_time)
        self:ApplyFriction(elapse_time)
    end
    MoveComponent.Update(self, elapse_time)
end

return _M
