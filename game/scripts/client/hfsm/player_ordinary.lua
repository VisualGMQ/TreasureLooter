local ClientWorld = require("client.world")
local ClientAttackComponent = require("client.components.attack")
local HFSMUtil = require("client.hfsm.hfsm_util")

---@class PlayerOrdinaryNode
---@field _entity LogicEntity
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@return PlayerOrdinaryNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    return self
end

function _M:OnInit()
    local world = ClientWorld.GetInst()
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_interact_component then
        go.m_interact_component:SetHintFX(world.m_player_hint)
    end
end

function _M:OnUpdate()
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
                local dir = go.m_move_component:GetLastMoveDirection()
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

function _M:OnQuit()
end

return _M
