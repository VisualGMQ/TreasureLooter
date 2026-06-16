local _M = {}
_M.__index = _M

type ComponentData = {
    _gameobject:any
}

export type Component = typeof(_M) & ComponentData

function _M.new(gameobject:any)
    local self = {}::ComponentData
    self._gameobject = gameobject
    return setmetatable(self, _M)
end

function _M.GetGameObject(self:Component)
    return self._gameobject
end

function _M.OnQuit(self:Component)
end

return _M