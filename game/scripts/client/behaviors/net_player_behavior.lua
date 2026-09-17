local ClientGameObjectBehavior = require("client.gameobject_behavior")
local ClientWorld = require("client.world")

---@class ClientMoveReconcilation
---@field seq number
---@field disp Vec2

---@class ClientNetPlayerBehavior : ClientGameObjectBehavior
---@field private _move_packet_seq number
---@field private _last_acked number
---@field private _reconcilation_list ClientMoveReconcilation[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity LogicEntity
---@return ClientNetPlayerBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self ClientNetPlayerBehavior
    self._move_packet_seq = 1
    self._last_acked = 0
    self._reconcilation_list = {}
    return self
end

function _M:OnInit()
    local ctx = TL_Client.GetContext()
    ctx:GetEventSystem():AddNetMsg_MoveEvent(function(id, peer, payload)
        self:onMoveNetEvent(peer, payload)
    end)
end

---@param peer UDPPeer
---@param move ProtoMove
---@private
function _M:onMoveNetEvent(peer, move)
    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- the server broadcasts every player's move; only reconcile our own entity
    if move:m_net_id() ~= go:GetNetID() then
        return
    end

    local seq = move:m_seq()
    if seq <= self._last_acked then
        return
    end
    self._last_acked = seq

    -- reset to server position
    local net_target = move:m_target()
    local target = TL_Common.Vec2(net_target:m_x(), net_target:m_y())
    local offset = target - go.m_transform:GetGlobalPosition()
    if offset:LengthSquared() > 0 then
        go.m_move_component:SetMoveDisp(offset)
    end
    go.m_move_component:Update()

    -- do reconciliation
    while #self._reconcilation_list > 0
        and self._reconcilation_list[1].seq <= seq do
        table.remove(self._reconcilation_list, 1)
    end

    for _, elem in ipairs(self._reconcilation_list) do
        go.m_move_component:SetMoveDisp(elem.disp)
        go.m_move_component:Update()
    end
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    local ctx = TL_Client.GetContext()
    local host = ctx:GetNetHost()
    if not host then
        return
    end

    local input_manager = ctx:GetInputManager()
    local action = TL_Proto.ClientAction()

    local action_type = nil
    local attack = input_manager:GetAction("Attack")
    if attack:IsPressed(0) then
        action_type = TL_Schema.ClientActionType.ClientActionType_Attack
    else
        local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
        if axises:LengthSquared() ~= 0 then
            local move_component = self.m_gameobject.m_move_component
            if not move_component then
                return
            end
            move_component:SetDir(axises)
            local disp = move_component:GetVelocity() *  elapse_time
            -- send move packet to net
            action_type = TL_Schema.ClientActionType.ClientActionType_Move
            local net_disp = TL_Proto.NetVec2()
            net_disp:set_m_x(disp.x)
            net_disp:set_m_y(disp.y)
            action:set_m_move_disp(net_disp)
            action:set_m_seq(self._move_packet_seq)

            -- client move first
            move_component:SetMoveDisp(disp)
            move_component:Update(elapse_time)

            -- remember it so it can be replayed until the server acks it
            local reconciliation = {
                seq = self._move_packet_seq,
                disp = disp
            }
            table.insert(self._reconcilation_list, reconciliation)

            self._move_packet_seq = self._move_packet_seq + 1
        end
    end

    if action_type ~= nil then
        action:set_m_action(action_type)
        local net_msg = TL_Proto.NetMsg()
        net_msg:set_m_client_action(action)
        host:Send(ctx:GetNetPeer(), net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

-- Follow the character in the render phase so the camera and the sprite sample
-- the exact same (global) transform value; updating in OnUpdate would read a
-- stale global matrix from before RelationshipManager::Update and cause jitter.
function _M:OnRender()
    local go = self:GetGameObject()
    if go and go.m_transform then
        TL_Client.GetContext():GetCamera():MoveTo(go.m_transform:GetGlobalPosition())
    end
end

return _M
