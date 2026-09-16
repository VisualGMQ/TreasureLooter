local ServerMoveComponent = require("server.components.move")
local GameObject = require("common.gameobject")

---@class ServerGameObjectDefinition
---@field m_did DID
---@field m_move_component_definition ServerMoveComponentDefinition|nil

---@class ServerGameObject : GameObject
---@field m_move_component ServerMoveComponent|nil
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = GameObject })

---@param entity LogicEntity
---@param definition ServerGameObjectDefinition
---@return ServerGameObject
function _M.new(entity, definition)
    local self = GameObject.new(entity, definition.m_did)
    ---@cast self ServerGameObject
    if definition.m_move_component_definition then
        self.m_move_component = ServerMoveComponent.new(self, definition.m_move_component_definition)
    end
    return setmetatable(self, _M)
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    if self.m_move_component then
        self.m_move_component:Update(elapse_time)
    end
end

function _M:OnQuit()
    if self.m_move_component then
        self.m_move_component:OnQuit()
    end
end

return _M
