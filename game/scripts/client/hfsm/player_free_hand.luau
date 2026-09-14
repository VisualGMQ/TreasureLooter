local ClientGameObject = require("@client/gameobject")
local HFSMUtil = require("@client/hfsm/hfsm_util")

local _M = {}
_M.__index = _M

export type PlayerFreeHandNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerFreeHandNode
    local self = {} :: PlayerFreeHandNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnUpdate(self: PlayerFreeHandNode)
    local ctx = TL_Client.GetContext()
    local go = HFSMUtil.GetGameObject(self._entity) :: ClientGameObject.ClientGameObject
    if not go then
        return
    end
    local input_manager = ctx:GetInputManager()

    local action = input_manager:GetAction("Attack")
    if action:IsPressed(0) then
        local weapon_script = go.m_weapon_component and go.m_weapon_component:GetScript()
        local weapon_go = weapon_script and weapon_script.m_gameobject
        if weapon_go and weapon_go.m_attack_component then
            weapon_go.m_attack_component:Attack()
        end
    end
end

function _M.OnQuit(self: PlayerFreeHandNode)
end

return _M
