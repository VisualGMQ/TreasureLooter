local ClientGameObjectBehavior = require("@client/gameobject_behavior")
local ClientGameObject = require("@client/gameobject")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

export type ClientNetPlayerBehavior = typeof(_M) & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity: Entity): ClientNetPlayerBehavior
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    return self
end

function _M.OnInit(self: ClientNetPlayerBehavior)
    local ctx = TL_Client.GetContext()
    ctx:GetEventSystem():AddNetMsg_MoveEvent(function(id, payload)
        local go = self:GetGameObject() :: ClientGameObject.ClientGameObject
        if not go or not go.m_move_component then
            return
        end

        local position = payload:m_position()
        local pos = TL_Common.Vec2(position:m_x(), position:m_y())
        local offset = pos - go.m_transform:GetGlobalPosition()
        local offset_squared_len = offset:LengthSquared()

        if offset_squared_len > 0 then
            local offset_len = math.sqrt(offset_squared_len)
            go.m_move_component:SetDir(offset / offset_len)
            go.m_move_component:ChangeSpeed(offset_len)
            go.m_move_component:Update(1)
        else
            go.m_move_component:StopMove()
        end
    end)
end

function _M.OnUpdate(self: ClientNetPlayerBehavior, elapse_time: TimeType)
    local ctx = TL_Client.GetContext()
    local host = ctx:GetNetHost()
    if not host then
        return
    end

    local input_manager = ctx:GetInputManager()
    local action = TL_Proto.ClientAction()

    local attack = input_manager:GetAction("Attack")
    if attack:IsPressed(0) then
        action:set_m_action(TL_Schema.ClientActionType.ClientActionType_Attack)
    else
        local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
        local direction = TL_Proto.NetVec2()
        direction:set_m_x(axises.x)
        direction:set_m_y(axises.y)
        action:set_m_action(TL_Schema.ClientActionType.ClientActionType_Move)
        action:set_m_move_dir(direction)
    end

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_client_action(action)
    host:Send(ctx:GetNetPeer(), net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
end

-- Follow the character in the render phase so the camera and the sprite sample
-- the exact same (global) transform value; updating in OnUpdate would read a
-- stale global matrix from before RelationshipManager::Update and cause jitter.
function _M.OnRender(self: ClientNetPlayerBehavior)
    local go = self:GetGameObject()
    if go and go.m_transform then
        TL_Client.GetContext():GetCamera():MoveTo(go.m_transform:GetGlobalPosition())
    end
end

return _M
