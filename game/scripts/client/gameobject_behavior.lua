local ClientGameObject = require("client.gameobject")
local ScriptBehavior = require("common.script_behavior")

---@class ClientGameObjectBehavior : ScriptBehavior
---@field m_gameobject ClientGameObject
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ScriptBehavior })

---@param entity LogicEntity
---@return ClientGameObjectBehavior
function _M.new(entity)
    local self = ScriptBehavior.new(entity)
    ---@cast self ClientGameObjectBehavior
    return setmetatable(self, _M)
end

---@param definition ClientGameObjectDefinition
function _M:initGameObject(definition)
    self.m_gameobject = ClientGameObject.new(self:GetEntity(), definition)
end

---@return ClientGameObject
function _M:GetGameObject()
    return self.m_gameobject
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    if self.m_gameobject then
        self.m_gameobject:OnUpdate(elapse_time)
    end
end

function _M:OnQuit()
    if self.m_gameobject then
        self.m_gameobject:OnQuit()
    end
end

return _M
