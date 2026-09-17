local World = require("common.world")
local ServerCreation = require("server.creation")

-- The match is a free-for-all of at most this many players.
local k_max_players = 2
-- Minimum number of players still in the round for it to be worth playing.
-- The first countdown only arms with a full lobby (4/4), but a disconnection
-- before the countdown must not freeze the round forever: the survivors
-- re-arm the countdown (see `tryArmCountdown`). With a single survivor the
-- match is over instead and that player wins by default (see `checkWin`).
local k_min_players_for_match = 2
-- How long the players are still frozen after every one of them is ready.
local k_countdown = 3.0
-- Delay between the `Win` broadcast and the end of the round.
local k_post_win_disconnect_delay = 5.0
-- Spawn points are named `player_spawn_point1` .. `player_spawn_point4` by the
-- map, one per possible player.
local k_player_spawn_point_prefix = "player_spawn_point"
local k_player_script = TL_Common.Path("scripts/server/behaviors/net_player_behavior.lua")

--- A player that joined the lobby but whose entity is not created yet. The
--- entities are created all at once when the match starts (4 players joined).
---@class PendingPlayer
---@field m_did number
---@field m_spawn_point_name string
---@field m_position Vec2

---@class ServerWorld : World
---@field _replicate_peers table<NetID, ServerGameObject>
--- The players that are out of the match (killed or disconnected).
---@field private _dead table<NetID, boolean>
--- The number of peers that ever spawned a player. A win requires at least two
--- of them, otherwise a single connected player would win alone.
---@field private _spawned_peer_count number
---@field private _win_sent boolean
--- The spawn point assigned to each joined peer (`net_id -> spawn_point_name`).
---@field private _spawn_assignments table<NetID, string>
--- The pending spawn request of each joined peer, waiting for the match start.
---@field private _pending_players table<NetID, PendingPlayer>
--- The peers that finished loading the level, waiting for the others.
---@field private _prepared table<NetID, boolean>
--- Whether the match started, i.e. the player entities were created.
---@field private _game_started boolean
--- The server time from which player inputs are accepted, 0 while not armed.
---@field private _start_time number
--- Seconds left before the round is torn down after a win, 0 when inactive.
---@field private _end_countdown number
--- Set while the round is being torn down, so the disconnect events it causes
--- are ignored instead of broadcasting kills.
---@field private _resetting boolean
--- The last lobby count broadcast to the clients, i.e. the number of peers in
--- `_spawn_assignments` at the time of the last `PlayerJoined`. Used to skip
--- re-broadcasting an unchanged count (the clients are idempotent, but the
--- message is only meaningful when the lobby population changes).
---@field private _last_lobby_count number
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

---@return ServerWorld
function _M.new()
    local self = setmetatable({}, _M)
    ---@cast self ServerWorld
    self._replicate_peers = {}
    self._dead = {}
    self._spawned_peer_count = 0
    self._win_sent = false
    self._spawn_assignments = {}
    self._pending_players = {}
    self._prepared = {}
    self._game_started = false
    self._start_time = 0
    self._end_countdown = 0
    self._resetting = false
    self._last_lobby_count = 0
    return self
end

--- Drive the post-win countdown. Called every frame by the server game entry
--- (`ServerGameEntry:OnUpdate`), so no timer is needed.
---@param elapse_time TimeType
function _M:Update(elapse_time)
    if self._end_countdown <= 0 then
        return
    end

    self._end_countdown = self._end_countdown - elapse_time
    if self._end_countdown <= 0 then
        self._end_countdown = 0
        self:resetRound()
    end
end

--- The number of players currently in the lobby (joined, alive or not).
---@return number
function _M:getJoinedCount()
    local count = 0
    for _ in pairs(self._spawn_assignments) do
        count = count + 1
    end
    return count
end

--- Whether player inputs may be applied and replicated right now. Inputs are
--- dropped during the lobby phase and the pre-game countdown.
---@return boolean
function _M:CanAcceptPlayerInput()
    if not self._game_started or self._start_time <= 0 then
        return false
    end

    local now = TL_Server.GetContext():GetTime():GetCurrentTime()
    return now >= self._start_time
end

---@private
---@param spawn_point_name string
---@return boolean
function _M:isSpawnPointTaken(spawn_point_name)
    for _, name in pairs(self._spawn_assignments) do
        if name == spawn_point_name then
            return true
        end
    end
    return false
end

--- The lowest free `player_spawn_pointN` of the map, so the players are spread
--- over the map by join order.
---@private
---@return string? spawn_point_name
function _M:findFreeSpawnPointName()
    local spawn_points = self.m_spawn_points or {}
    for i = 1, k_max_players do
        local name = k_player_spawn_point_prefix .. i
        if spawn_points[name] and not self:isSpawnPointTaken(name) then
            return name
        end
    end
    return nil
end

---@param net_id NetID
---@param go ServerGameObject
function _M:AddPeer(net_id, go)
    if not self._replicate_peers[net_id] then
        self._spawned_peer_count = self._spawned_peer_count + 1
    end
    self._replicate_peers[net_id] = go
end

--- @brief remove peer from world and scene
---@param net_id NetID
function _M:RemovePeer(net_id)
    local go = self._replicate_peers[net_id]
    if not go then
        return
    end
    local entity = go:GetEntity()
    local ctx = TL_Server.GetContext()
    local relationship_mgr = ctx:GetRelationshipManager()
    local relationship = relationship_mgr:Get(entity)
    if relationship then
        relationship:RemoveFromParent()
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        scene:RemoveEntity(entity)
    end

    self._replicate_peers[net_id] = nil
end

--- Mark a player as out of the match and replicate its death. Idempotent: a
--- player that already died (or disconnected) is ignored. A player without a
--- live entity (never spawned, entity creation failed, already removed) is
--- ignored too, so a stale hit cannot broadcast a kill for a net id that is
--- not part of the round.
---@param net_id NetID
function _M:MarkDead(net_id)
    if not self._replicate_peers[net_id] then
        return
    end

    if self:markDeadInternal(net_id) then
        self:broadcastKill(net_id)
    end
    self:checkWin()
end

---@private
---@param net_id NetID
---@return boolean changed
function _M:markDeadInternal(net_id)
    if self._dead[net_id] then
        return false
    end
    self._dead[net_id] = true
    return true
end

--- Send a message to every peer that is a player of the current round, i.e.
--- the peers that reserved a lobby slot.
---
--- `UDPHost:Broadcast` iterates every peer known to the net host, what must be
--- avoided here: a peer that connected but never joined (a full lobby
--- rejection) and, above all, a peer the server disconnected while resetting
--- the round stays registered for a while and sending to it fails with a
--- `send packet ... failed` error. Peers that are not in `_spawn_assignments`
--- are skipped, and `GetPeer` returns an invalid peer for the ones already
--- erased from the net host.
---@param net_msg ProtoNetMsg
function _M:broadcastToPlayers(net_msg)
    local host = TL_Server.GetContext():GetNetHost()
    if not host then
        return
    end

    local flags = TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable)
    for net_id in pairs(self._spawn_assignments) do
        local peer = host:GetPeer(net_id)
        if peer and peer:IsValid() then
            host:Send(peer, net_msg, 0, flags)
        end
    end
end

---@private
---@param net_id NetID
function _M:broadcastKill(net_id)
    local kill_msg = TL_Proto.Kill()
    kill_msg:set_m_net_id(net_id)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_kill(kill_msg)

    self:broadcastToPlayers(net_msg)
end

--- Broadcast the win to the last player standing, only once per match. A win
--- is only checked once the match started (4 players joined and their entities
--- were created).
---@private
function _M:checkWin()
    if self._win_sent or not self._game_started then
        return
    end

    local alive_count = 0
    local alive_net_id = nil
    for net_id in pairs(self._replicate_peers) do
        if not self._dead[net_id] then
            alive_count = alive_count + 1
            alive_net_id = net_id
        end
    end

    if alive_count ~= 1 or alive_net_id == nil or self._spawned_peer_count < 2 then
        return
    end

    self._win_sent = true

    local win = TL_Proto.Win()
    win:set_m_net_id(alive_net_id)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_win(win)

    self:broadcastToPlayers(net_msg)
    TL_Server.GetContext():Log("player ", alive_net_id, " wins, broadcast Win")

    -- The round is over: let the clients show the result, then reset the server
    -- so a new match can start.
    self:scheduleRoundReset()
end

function _M:RegisterNetEventHandler()
    local ctx = TL_Server:GetContext()
    local event_system = ctx:GetEventSystem()
    event_system:AddNetMsg_SpawnPlayerRequestEvent(function(id, peer, payload)
        self:onSpawnPlayerRequest(peer, payload)
    end)
    event_system:AddNetMsg_ConnectEvent(function(id, peer, payload)
        self:onNetConnect(peer)
    end)
    event_system:AddNetMsg_DisconnectEvent(function(id, peer, payload)
        self:onNetDisconnect(peer)
    end)
    event_system:AddNetMsg_PrepareGameFinishEvent(function(id, peer, payload)
        self:onPrepareGameFinish(peer, payload)
    end)
end

function _M:onNetConnect(peer)
    self:replicateWorldToNewPeer(peer)
end

--- A `SpawnPlayerRequest` joins the lobby: the spawn point is reserved and the
--- player entity is created only when the match starts (see `startGame`). When
--- the lobby is already full the peer is rejected and disconnected.
---@param peer UDPPeer
---@param payload ProtoSpawnPlayerRequest
function _M:onSpawnPlayerRequest(peer, payload)
    local ctx = TL_Server.GetContext()
    local net_id = peer:GetID()

    -- Net id 0 is `UDPPeer.InvalidID`: never answer (nor send to) such a peer.
    if net_id == 0 then
        return
    end

    -- A peer that already joined keeps its spawn point: ignore duplicates.
    if self._spawn_assignments[net_id] then
        return
    end

    -- A round is already running: a new peer has no player entity to control
    -- (the characters are created all at once by `startGame`), so it is
    -- rejected like a full lobby and can join the next match.
    if self._game_started then
        self:rejectPeer(peer)
        return
    end

    if self:getJoinedCount() >= k_max_players then
        self:rejectPeer(peer)
        return
    end

    local spawn_point_name = self:findFreeSpawnPointName()
    if not spawn_point_name then
        ctx:Log("SpawnPlayerRequest: no free spawn point")
        self:rejectPeer(peer)
        return
    end

    local spawn_point = self.m_spawn_points[spawn_point_name]
    if not spawn_point then
        ctx:Log("SpawnPlayerRequest: can't find spawn point ", spawn_point_name)
        return
    end

    local did = payload:m_did()

    -- The net id may be reused by a later connection, so drop any stale state.
    self._dead[net_id] = nil
    self._spawn_assignments[net_id] = spawn_point_name
    self._pending_players[net_id] = {
        m_did = did,
        m_spawn_point_name = spawn_point_name,
        m_position = spawn_point.m_position,
    }

    ctx:Log("player ", net_id, " joined (", self:getJoinedCount(), "/",
        k_max_players, ") spawn ", spawn_point_name)

    self:broadcastPlayerJoined()

    -- Unicast the reservation so the peer knows its net id and spawn position
    -- while it waits in the lobby.
    local host = ctx:GetNetHost()
    if host then
        host:Send(peer, self:buildSpawnPlayerReplyMsg(net_id, did, spawn_point.m_position),
            0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end

    if self:getJoinedCount() >= k_max_players then
        self:startGame()
    end
end

--- Tell the peer the server is full and drop it. No spawn point is reserved.
---@private
---@param peer UDPPeer
function _M:rejectPeer(peer)
    local ctx = TL_Server.GetContext()
    ctx:Log("server full, reject peer ", peer:GetID())

    local host = ctx:GetNetHost()
    if host then
        local full = TL_Proto.ServerFull()
        local msg = TL_Proto.NetMsg()
        msg:set_m_server_full(full)
        host:Send(peer, msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))

        -- Send() only queues the packet; it has to be on the wire before the
        -- connection is dropped, otherwise the client never learns it was
        -- rejected.
        host:Flush()

        -- A peer taken from a net event only exposes its const methods
        -- (`GetID`, `GetRTT`, `IsValid`), so the mutable one is fetched from
        -- the host to actually drop the connection.
        local target = host:GetPeer(peer:GetID())
        if target and target:IsValid() then
            target:Disconnect()
        end
    end
end

--- Broadcast how many players are currently in the lobby so the clients update
--- the waiting UI. Called on every lobby population change (a join as well as a
--- disconnect before the match starts). Idempotent: an unchanged count is not
--- sent again.
---@private
function _M:broadcastPlayerJoined()
    local count = self:getJoinedCount()
    if count == self._last_lobby_count then
        return
    end

    local ctx = TL_Server.GetContext()
    self._last_lobby_count = count

    local joined = TL_Proto.PlayerJoined()
    joined:set_m_count(count)
    joined:set_m_max(k_max_players)

    local msg = TL_Proto.NetMsg()
    msg:set_m_player_joined(joined)
    self:broadcastToPlayers(msg)

    ctx:Log("lobby count ", count, "/", k_max_players)
end

---@private
---@param net_id NetID
---@param did number
---@param position Vec2
---@return ProtoNetMsg
function _M:buildSpawnPlayerReplyMsg(net_id, did, position)
    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local reply = TL_Proto.SpawnPlayerReply()
    reply:set_m_net_id(net_id)
    reply:set_m_did(did)
    reply:set_m_position(net_position)

    local msg = TL_Proto.NetMsg()
    msg:set_m_spawn_player_reply(reply)
    return msg
end

--- The lobby is full: create every player entity on its reserved spawn point
--- and broadcast every `SpawnPlayerReply` so all the clients create the
--- characters (local and remote) at the same time.
---@private
function _M:startGame()
    if self._game_started then
        return
    end

    local ctx = TL_Server.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        -- Stay in the lobby (a later disconnect or a new request re-tries the
        -- start) instead of flipping `_game_started` with no entity created.
        ctx:Log("start game failed: no current scene")
        return
    end

    self._game_started = true

    for net_id, pending in pairs(self._pending_players) do
        self:createPlayerEntity(scene, net_id, pending)
    end

    -- Every round player receives every spawn reply, so all the clients create
    -- the characters (local and remote) at the same time. Sent through
    -- `broadcastToPlayers` so a peer the server already dropped is skipped.
    for net_id, pending in pairs(self._pending_players) do
        self:broadcastToPlayers(
            self:buildSpawnPlayerReplyMsg(net_id, pending.m_did, pending.m_position))
    end

    -- Confirm the final lobby count (4/4) to every client. A no-op when the
    -- broadcast of the last join already reached them (see
    -- `broadcastPlayerJoined`), but it still covers a peer whose join
    -- broadcast was skipped because the net host was not ready yet.
    self:broadcastPlayerJoined()
    ctx:Log("game starts with ", self:getJoinedCount(), " players")

    -- Some clients may have finished loading before the match started.
    self:tryArmCountdown()
end

---@private
---@param scene Scene
---@param net_id NetID
---@param pending PendingPlayer
function _M:createPlayerEntity(scene, net_id, pending)
    local ctx = TL_Server.GetContext()

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = pending.m_did
    spawn_info.m_server_script = k_player_script
    spawn_info.m_spawn_point_name = pending.m_spawn_point_name

    local entity, go = ServerCreation.CreateCharacter(ServerCreation, scene, spawn_info,
        pending.m_position, net_id, self.m_object_definitions)
    if not go then
        ctx:Log("create player entity failed, net_id ", net_id)
        return
    end

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    if root_relationship then
        root_relationship:AddChild(entity)
    end

    self:AddPeer(net_id, go)
    ctx:Log("server spawned player by did ", pending.m_did, " net_id ", net_id,
        " at ", pending.m_spawn_point_name)
end

--- A client finished loading the level. Readiness is recorded even before the
--- match starts: a client loads the scene right after connecting, so its
--- `PrepareGameFinish` can arrive before the 4th player joins.
---@param peer UDPPeer
---@param payload ProtoPrepareGameFinish
function _M:onPrepareGameFinish(peer, payload)
    if self._start_time > 0 then
        return
    end

    local net_id = peer:GetID()
    if not self._spawn_assignments[net_id] then
        return
    end

    self._prepared[net_id] = true
    self:tryArmCountdown()
end

--- Arm the pre-game countdown and broadcast it once the match started and
--- every player still in the round is ready. Inputs stay ignored until it ends
--- (see `CanAcceptPlayerInput`).
---
--- Readiness is checked against the players *currently* in the round, not
--- against the full lobby: a client that disconnects before the countdown was
--- armed (its `PrepareGameFinish` may never arrive) must not freeze the match
--- forever. The survivors re-arm the countdown; if the disconnections left a
--- single player the round ends instead and that player wins by default.
---@private
function _M:tryArmCountdown()
    -- Never re-arm once the countdown is running or the round was won.
    if self._start_time > 0 or self._end_countdown > 0 or not self._game_started then
        return
    end

    local ready_count = 0
    for net_id in pairs(self._replicate_peers) do
        if not self._dead[net_id] then
            -- A player that is still loading blocks the countdown.
            if not self._prepared[net_id] then
                return
            end
            ready_count = ready_count + 1
        end
    end

    -- A single player cannot start a match; `checkWin` handles that case by
    -- closing the round.
    if ready_count < k_min_players_for_match then
        return
    end

    local ctx = TL_Server.GetContext()
    self._start_time = ctx:GetTime():GetCurrentTime() + k_countdown
    ctx:Log("all players prepared, countdown ", k_countdown)

    local countdown = TL_Proto.PrepareCountdown()
    countdown:set_m_countdown(k_countdown)

    local msg = TL_Proto.NetMsg()
    msg:set_m_prepare_countdown(countdown)
    self:broadcastToPlayers(msg)
end

--- Arm the post-win countdown (driven by `Update`).
---@private
function _M:scheduleRoundReset()
    if self._end_countdown > 0 then
        return
    end

    self._end_countdown = k_post_win_disconnect_delay
    TL_Server.GetContext():Log("game over, disconnecting all peers in ",
        k_post_win_disconnect_delay, "s")
end

--- Disconnect every joined peer, remove their entities and reset the lobby so
--- the next connections can start a fresh match.
---@private
function _M:resetRound()
    local ctx = TL_Server.GetContext()
    local host = ctx:GetNetHost()

    -- Ignore the disconnect events the `Disconnect` calls below will enqueue.
    self._resetting = true

    local net_ids = {}
    for net_id in pairs(self._replicate_peers) do
        table.insert(net_ids, net_id)
    end
    for net_id in pairs(self._spawn_assignments) do
        if not self._replicate_peers[net_id] then
            table.insert(net_ids, net_id)
        end
    end

    for _, net_id in ipairs(net_ids) do
        self:RemovePeer(net_id)
        local peer = host and host:GetPeer(net_id)
        if peer and peer:IsValid() then
            peer:Disconnect()
        end
    end

    self:resetLobbyState()
    self._resetting = false
    ctx:Log("round reset, ready for new connections")
end

--- Remove a peer from the lobby statistics and free its spawn point.
---@private
---@param net_id NetID
function _M:removeFromLobby(net_id)
    self._spawn_assignments[net_id] = nil
    self._pending_players[net_id] = nil
    self._prepared[net_id] = nil
    -- A player that leaves before the match starts is not part of the alive
    -- statistics, so its death flag is cleared too (net ids can be reused).
    if not self._game_started then
        self._dead[net_id] = nil
    end
end

--- Clear every piece of per-round state.
---@private
function _M:resetLobbyState()
    self._spawn_assignments = {}
    self._pending_players = {}
    self._prepared = {}
    self._dead = {}
    self._replicate_peers = {}
    self._spawned_peer_count = 0
    self._game_started = false
    self._start_time = 0
    self._win_sent = false
    self._end_countdown = 0
    self._last_lobby_count = 0
end

---@param peer UDPPeer
function _M:replicateWorldToNewPeer(peer)
    -- A running round rejects new players (see `onSpawnPlayerRequest`), so
    -- there is nothing worth replicating to them.
    if self._game_started then
        return
    end

    local ctx = TL_Server.GetContext()
    local host = ctx:GetNetHost()
    if not host then
        return
    end

    for net_id, go in pairs(self._replicate_peers) do
        if net_id == peer:GetID() then
            goto continue
        end
        local msg = TL_Proto.NetMsg()
        local spawn_msg = TL_Proto.SpawnPlayerReply()
        spawn_msg:set_m_did(go:GetDID())
        spawn_msg:set_m_net_id(go:GetNetID())

        local position = go.m_transform:GetGlobalPosition()
        local net_position = TL_Proto.NetVec2()
        net_position:set_m_x(position.x)
        net_position:set_m_y(position.y)
        spawn_msg:set_m_position(net_position)

        msg:set_m_spawn_player_reply(spawn_msg)
        -- only the newly connected peer needs the existing players
        host:Send(peer, msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))

        ::continue::
    end
end

---@param peer UDPPeer
function _M:onNetDisconnect(peer)
    local ctx = TL_Server.GetContext()
    local net_id = peer:GetID()

    -- Disconnect events are delivered one frame after the reset that caused
    -- them, so most of them arrive once the peer was already removed from
    -- every table. There is nothing left to clean nor to broadcast then.
    local was_spawned = self._replicate_peers[net_id] ~= nil
    if not was_spawned and not self._spawn_assignments[net_id] and
        not self._pending_players[net_id] and not self._prepared[net_id] then
        return
    end

    self:RemovePeer(net_id)

    if was_spawned then
        -- Keep telling the clients that the player is gone (the disconnected
        -- peer is already erased from the net host, so only the others receive
        -- it) and drop it from the alive count so the last player can win. A
        -- player that already died broadcast its kill when it died.
        if self:markDeadInternal(net_id) then
            self:broadcastKill(net_id)
        end
    end

    -- Free its lobby slot (this also clears the death flag when the match had
    -- not started yet).
    self:removeFromLobby(net_id)

    -- The round is being torn down: the kills and the win were already handled
    -- by the reset.
    if self._resetting then
        return
    end

    -- The lobby population changed: tell the remaining clients (a disconnect
    -- before the match starts must update their waiting UI, and after the start
    -- the count is kept consistent as well). An unchanged count is skipped.
    self:broadcastPlayerJoined()

    -- A disconnect before the countdown may leave fewer than 4 players: first
    -- let `checkWin` end the round when a single player is left, then re-arm
    -- the countdown on the survivors otherwise.
    self:checkWin()
    self:tryArmCountdown()

    -- Nobody left: reset the lobby so the next connections start a new match.
    -- Placed after the broadcast so a fresh connection always starts from the
    -- 0/4 baseline (`resetLobbyState` clears `_last_lobby_count` and any
    -- pending post-win countdown).
    if self:getJoinedCount() == 0 then
        self:resetLobbyState()
    end

    ctx:Log("peer ", net_id, " disconnected")
end

return _M
