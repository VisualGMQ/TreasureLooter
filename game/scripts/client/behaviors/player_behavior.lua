local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class PlayerBehavior : ClientGameObjectBehavior
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity Entity
---@return PlayerBehavior
function _M.new(entity)
    local self = ClientGameObjectBehavior.new(entity)
    ---@cast self PlayerBehavior
    return setmetatable(self, _M)
end

return _M
