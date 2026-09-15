local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class WorkbenchLogicData

---@class WorkbenchLogic : ClientGameObjectBehavior
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param gameobject any
---@return WorkbenchLogic
function _M.new(gameobject)
    local self = ClientGameObjectBehavior.new(gameobject)
    ---@cast self WorkbenchLogic
    return setmetatable(self, _M)
end

function _M:OnInit()
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)
end

return _M
