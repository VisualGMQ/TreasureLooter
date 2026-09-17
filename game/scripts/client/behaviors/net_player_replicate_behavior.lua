local ClientGameObjectBehavior = require("client.gameobject_behavior")
local ClientWorld = require("client.world")
local ClientGameObjectAccessor = require("client.go_accessor")

--- One received authoritative position for a remote player.
---@class NetSnapshot
---@field m_timestamp TimeType
---@field m_position Vec2

--- Behavior for a player owned by another client (created from the
--- SpawnPlayerReply broadcast).
---
--- Split of responsibilities:
---  - on packet: the physics (CCT) transform snaps to the authoritative
---    position, and the snapshot is buffered;
---  - OnRender: only the render-only (present) transform is interpolated
---    between the two snapshots straddling `now - interp_delay`. The physics
---    transform is never interpolated.
---@class ClientNetPlayerReplicateBehavior : ClientGameObjectBehavior
---@field private _snapshots NetSnapshot[]
---@field private _move_listener EventListenerID?
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
    self._move_listener = ctx:GetEventSystem():AddNetMsg_MoveEvent(
        function(id, peer, move)
            self:onMoveNetEvent(move)
        end)
end

-- Net listeners outlive the entity/script they were registered from, so a
-- stale one would keep writing into a character of the unloaded scene.
function _M:OnQuit()
    if not self._move_listener then
        return
    end

    TL_Client.GetContext():GetEventSystem():Remove(self._move_listener)
    self._move_listener = nil
end

---@param move ProtoMove
---@private
function _M:onMoveNetEvent(move)
    local go = self:GetGameObject()
    if not go then
        return
    end

    -- The listener can still fire in the frame the game scene is unloaded:
    -- never touch a character whose physics was already destroyed.
    if not TL_Client.GetContext():GetTransformManager():Has(go:GetEntity()) then
        return
    end

    -- the server broadcasts every player's move; only apply our own entity's
    if move:m_net_id() ~= go:GetNetID() then
        return
    end

    local target = move:m_target()
    local target_position = TL_Common.Vec2(target:m_x(), target:m_y())

    local move_component = go.m_move_component

    -- A remote player never receives a displacement: the packets are the only
    -- thing that tells it, so derive the facing/walk animation from how far the
    -- authoritative position moved since the previous packet.
    if move_component and #self._snapshots > 0 then
        local previous_position = self._snapshots[#self._snapshots].m_position
        local delta = target_position - previous_position
        if delta:LengthSquared() > 0.000001 then
            move_component:SetDir(delta:Normalize())
        else
            move_component:SetDir(TL_Common.Vec2.ZERO)
        end
    end

    if move_component then
        move_component:Teleport(target_position)
    else
        go.m_transform.m_position = target_position
    end

    table.insert(self._snapshots, {
        m_timestamp = TL_Client.GetContext():GetTime():GetCurrentTime(),
        m_position = target_position,
    })
end

--- Advance the move component for the animation only (the physics transform is
--- snapped by the packets, so it never gets a displacement here): it plays the
--- walk animation while the packets show movement and the idle frame otherwise.
---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)

    local go = self:GetGameObject()
    if go and go.m_move_component then
        go.m_move_component:Update(elapse_time)
    end
end

function _M:OnRender()
    local go = self:GetGameObject()
    if not go or #self._snapshots < 2 then
        return
    end

    local ctx = TL_Client.GetContext()
    -- The entity can be destroyed with the scene while this script is still
    -- alive for a frame: never touch a present entity that is gone.
    if not ctx:GetTransformManager():Has(go:GetEntity()) then
        return
    end

    local world = ClientWorld.GetInst()
    ---@cast world ClientWorld

    local now = ctx:GetTime():GetCurrentTime()
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

    local present =
        ClientGameObjectAccessor.GetPresentTransform(go:GetEntity())
    if present then
        present.m_position = position
        -- refresh the whole present subtree: the children (weapon, ...) were
        -- computed in the render sync with the previous parent matrix
        present:UpdateHierarchy()
    end
end

return _M
