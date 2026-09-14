local ClientWorld = require("@client/world")
local ClientAttackComponent = require("@client/components/attack")
local HFSMUtil = require("@client/hfsm/hfsm_util")

local _M = {}
_M.__index = _M

export type PlayerOrdinaryNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerOrdinaryNode
    local self = {} :: PlayerOrdinaryNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnInit(self: PlayerOrdinaryNode)
    local world = ClientWorld.GetInst() :: ClientWorld.ClientWorld
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_interact_component then
        go.m_interact_component:SetHintFX(world.m_player_hint)
    end
end

function _M.OnUpdate(self: PlayerOrdinaryNode)
    local ctx = TL_Client.GetContext()
    local elapse_time = HFSMUtil.GetElapseTime()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end

    local input_manager = ctx:GetInputManager()
    local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)

    if go.m_move_component then
        go.m_move_component:SetDir(axises)
        go.m_move_component:Update(elapse_time)
    end

    if go.m_weapon_component and go.m_move_component then
        local weapon_script = go.m_weapon_component:GetScript()
        if weapon_script and weapon_script.m_gameobject then
            local weapon_go = weapon_script.m_gameobject
            if weapon_go.m_attack_component then
                local dir = go.m_move_component:GetMoveDirection()
                if dir:LengthSquared() == 0 then
                    dir = ClientAttackComponent.k_init_forward_dir
                end
                weapon_go.m_attack_component:SetAimDir(dir)
            end
        end
    end

    if go.m_interact_component then
        go.m_interact_component:Update(elapse_time)
    end
end

function _M.OnQuit(self: PlayerOrdinaryNode)
end

return _M
