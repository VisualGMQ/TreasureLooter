local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}

function _M.GetGameObject(entity: Entity)
    local ctx = TL_Client.GetContext()
    local behavior = ctx:GetScriptManager():Get(entity) :: ClientGameObjectBehavior.ClientGameObjectBehavior
    if not behavior then
        return nil
    end
    return behavior.m_gameobject
end

function _M.ChangeState(entity: Entity, state_id: number)
    local ctx = TL_Client.GetContext()
    local hfsm = ctx:GetHFSMComponentManager():Get(entity)
    if hfsm then
        hfsm:ChangeState(state_id)
    end
end

function _M.GetElapseTime(): number
    return TL_Client.GetContext():GetTime():GetElapseTime()
end

return _M
