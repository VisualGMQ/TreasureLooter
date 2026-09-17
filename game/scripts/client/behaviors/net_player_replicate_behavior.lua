local ClientGameObjectBehavior = require("client.gameobject_behavior")
local ClientWorld = require("client.world")

--- One received authoritative position for a remote player.
---@class NetSnapshot
---@field m_timestamp TimeType
---@field m_position Vec2

--- Behavior for a player owned by another client (created from the
--- SpawnPlayerReply broadcast). The server only sends a few positions per
--- second, so the character is rendered `interp_delay` behind real time and
--- interpolated between the two newest snapshots every frame.
---@class ClientNetPlayerReplicateBehavior : ClientGameObjectBehavior
---@field private _snapshots NetSnapshot[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity LogicEntity
---@return ClientNetPlayerReplicateBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self ClientNetPlayerReplicateBehavior
    self._snapshots = {}
    return self
end

function _M:OnInit()
    local ctx = TL_Client.GetContext()
    ctx:GetEventSystem():AddNetMsg_MoveEvent(function(id, peer, move)
        self:onMoveNetEvent(move)
    end)
end

--- Buffer the snapshot only. The position is computed in OnUpdate so it is
--- updated every render frame instead of once per received packet.
---@param move ProtoMove
---@private
function _M:onMoveNetEvent(move)
    local go = self:GetGameObject()
    if not go then
        return
    end

    -- the server broadcasts every player's move; only buffer our own entity's
    if move:m_net_id() ~= go:GetNetID() then
        return
    end

    local target = move:m_target()
    table.insert(self._snapshots, {
        -- local arrival time: avoids depending on the server wall clock
        m_timestamp = TL_Client.GetContext():GetTime():GetCurrentTime(),
        m_position = TL_Common.Vec2(target:m_x(), target:m_y()),
    })
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)

    local go = self:GetGameObject()
    if not go or #self._snapshots < 2 then
        return
    end

    local now = TL_Client.GetContext():GetTime():GetCurrentTime()
    local world = ClientWorld.GetInst()
    ---@cast world ClientWorld
    local render_time = now - world:GetNetInterpDelay()

    -- drop the snapshots already consumed, keeping one before render_time
    while #self._snapshots >= 3 and self._snapshots[2].m_timestamp <= render_time do
        table.remove(self._snapshots, 1)
    end

    local before = self._snapshots[1]
    local after = self._snapshots[2]

    local position
    if render_time <= before.m_timestamp then
        -- not enough history yet: hold the oldest snapshot
        position = before.m_position
    elseif render_time >= after.m_timestamp then
        -- starved (packet late/lost): hold the newest snapshot, no extrapolation
        position = after.m_position
    else
        local interval = after.m_timestamp - before.m_timestamp
        if interval <= 0 then
            position = after.m_position
        else
            local alpha = (render_time - before.m_timestamp) / interval
            position = before.m_position +
                       (after.m_position - before.m_position) * alpha
        end
    end

    local move_component = go.m_move_component
    if move_component then
        move_component:Teleport(position)
    else
        go.m_transform.m_position = position
    end
end

return _M
