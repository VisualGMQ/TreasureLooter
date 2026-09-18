local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class HFSMUtil
local _M = {}

---@param entity LogicEntity
---@return any
function _M.GetGameObject(entity)
    local ctx = TL_Client.GetContext()
    local behavior = ctx:GetScriptManager():Get(entity)
    if not behavior then
        return nil
    end
    return behavior.m_gameobject
end

---@param entity LogicEntity
---@param state_id integer
function _M.ChangeState(entity, state_id)
    local ctx = TL_Client.GetContext()
    local hfsm = ctx:GetHFSMComponentManager():Get(entity)
    if hfsm then
        hfsm:ChangeState(state_id)
    end
end

---@return number
function _M.GetElapseTime()
    return TL_Client.GetContext():GetTime():GetElapseTime()
end

return _M
