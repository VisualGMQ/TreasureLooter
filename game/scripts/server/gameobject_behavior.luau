local ServerGameObject = require("@server/gameobject")
local ScriptBehavior = require("@common/script_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ScriptBehavior })

type ServerGameObjectBehaviorData = {
    m_gameobject:ServerGameObject.ServerGameObject,
}
export type ServerGameObjectBehavior = typeof(setmetatable({}::ServerGameObjectBehaviorData, _M)) & ScriptBehavior.ScriptBehavior

function _M.new(entity:Entity):ServerGameObjectBehavior
    local self = ScriptBehavior.new(entity)
    return setmetatable(self, _M)
end

function _M.initGameObject(self:ServerGameObjectBehavior, definition:ServerGameObject.ServerGameObjectDefinition)
    self.m_gameobject = ServerGameObject.new(self:GetEntity(), definition)
end

function _M.GetGameObject(self:ServerGameObjectBehavior)
    return self.m_gameobject
end

function _M.setLogicComponent(self:ServerGameObjectBehavior, logic_script: Path)
    local module_name = TL_Common.PathToModuleName(logic_script)
    local logic_module = require(module_name)
    local go = self.m_gameobject
    if go and go.m_logic_component then
        go.m_logic_component:OnLogicTerminate()
    end
    go.m_logic_component = logic_module.new(go)
    go.m_logic_component:OnInit()
end

function _M.OnUpdate(self:ServerGameObjectBehavior, elapse_time:TimeType)
    if self.m_gameobject then
        self.m_gameobject:OnUpdate(elapse_time)
    end
end

function _M.OnQuit(self:ServerGameObjectBehavior)
    if self.m_gameobject then
        self.m_gameobject:OnQuit()
    end
end

return _M
