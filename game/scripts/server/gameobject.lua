local ServerMoveComponent = require("server.components.move")
local ServerHpComponent = require("server.components.hp")
local GameObject = require("common.gameobject")

---@class ServerGameObjectDefinition
---@field m_did DID
---@field m_net_id number
---@field m_move_component_definition ServerMoveComponentDefinition|nil
---@field m_hp_component_definition ServerHpComponentDefinition|nil

---@class ServerGameObject : GameObject
---@field private _net_id number
---@field m_move_component ServerMoveComponent|nil
---@field m_hp_component ServerHpComponent|nil
---@field m_logic_component any
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
    if definition.m_hp_component_definition then
        self.m_hp_component = ServerHpComponent.new(self, definition.m_hp_component_definition)
    end
    self._net_id = definition.m_net_id
    return setmetatable(self, _M)
end

function _M:GetNetID()
    return self._net_id
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
    if self.m_hp_component then
        self.m_hp_component:OnQuit()
    end
end

return _M
