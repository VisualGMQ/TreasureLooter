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

function _M.GetGameObject(self:ClientGameObjectBehavior)
    return self.m_gameobject
end

function _M.setLogicComponent(self:ClientGameObjectBehavior, logic_script: Path)
    local module_name = TL_Common.PathToModuleName(logic_script)
    local logic_module = require(module_name)
    local go = self.m_gameobject
    if go.m_logic_component then
        go.m_logic_component:OnLogicTerminate()
    end
    go.m_logic_component = logic_module.new(go)
    go.m_logic_component:OnInit()
end

function _M.OnUpdate(self:ClientGameObjectBehavior, elapse_time:TimeType)
    if self.m_gameobject then
        self.m_gameobject:OnUpdate(elapse_time)
    end
end

function _M.OnQuit(self:ClientGameObjectBehavior)
    if self.m_gameobject then
        self.m_gameobject:OnQuit()
    end
end

return _M
