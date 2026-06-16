local ClientGameObject = require("@client/gameobject")
local ScriptBehavior = require("@common/script_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ScriptBehavior })

type ClientGameObjectBehaviorData = {
    m_gameobject:ClientGameObject.ClientGameObject,
}
export type ClientGameObjectBehavior = typeof(_M) & ClientGameObjectBehaviorData & ScriptBehavior.ScriptBehavior

function _M.new(entity:Entity):ClientGameObjectBehavior
    local self = ScriptBehavior.new(entity)
    return setmetatable(self, _M)
end

function _M.initGameObject(self:ClientGameObjectBehavior, definition:ClientGameObject.ClientGameObjectDefinition)
    self.m_gameobject = ClientGameObject.new(self:GetEntity(), definition)
end

function _M.OnQuit(self:ClientGameObjectBehavior)
    self.m_gameobject:OnQuit()
end

return _M
