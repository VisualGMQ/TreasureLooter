local ServerGameObject = require("server.gameobject")
local ScriptBehavior = require("common.script_behavior")

--- Convert a script file path into its `require` module name
--- (e.g. "scripts/server/behaviors/net_player_behavior.lua" -> "server.behaviors.net_player_behavior").
---@param path Path
---@return string
local function path_to_module_name(path)
    local s = path:string()
    s = s:gsub("\\", "/")
    local idx = s:find("scripts/", 1, true)
    if idx then
        s = s:sub(idx + #"scripts/")
    end
    s = s:gsub("%.lua$", "")
    s = s:gsub("/", ".")
    return s
end

---@class ServerGameObjectBehaviorData
---@field m_gameobject ServerGameObject

---@class ServerGameObjectBehavior : ScriptBehavior
---@field m_gameobject ServerGameObject
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ScriptBehavior })

---@param entity LogicEntity
---@return ServerGameObjectBehavior
function _M.new(entity)
    local self = ScriptBehavior.new(entity)
    ---@cast self ServerGameObjectBehavior
    return setmetatable(self, _M)
end

---@param definition ServerGameObjectDefinition
function _M:initGameObject(definition)
    self.m_gameobject = ServerGameObject.new(self:GetEntity(), definition)
end

---@return ServerGameObject
function _M:GetGameObject()
    return self.m_gameobject
end

---@param logic_script Path
function _M:setLogicComponent(logic_script)
    local module_name = path_to_module_name(logic_script)
    local logic_module = require(module_name)
    local go = self.m_gameobject
    if go and go.m_logic_component then
        go.m_logic_component:OnLogicTerminate()
    end
    go.m_logic_component = logic_module.new(go)
    go.m_logic_component:OnInit()
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
