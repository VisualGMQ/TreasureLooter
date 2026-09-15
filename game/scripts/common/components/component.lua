---@class Component
---@field _gameobject any
local _M = {}
_M.__index = _M

---@param gameobject any
---@return Component
function _M.new(gameobject)
    local self = setmetatable({}, _M)
    self._gameobject = gameobject
    return self
end

---@return any
function _M:GetGameObject()
    return self._gameobject
end

function _M:OnQuit()
end

return _M
