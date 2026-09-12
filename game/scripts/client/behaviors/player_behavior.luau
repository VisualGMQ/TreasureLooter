local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, ClientGameObjectBehavior)

export type PlayerBehavior = typeof(_M) & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity: Entity): PlayerBehavior
    local self = ClientGameObjectBehavior.new(entity) :: PlayerBehavior
    return setmetatable(self, _M)
end

return _M
