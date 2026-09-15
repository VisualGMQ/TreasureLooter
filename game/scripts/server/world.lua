local World = require("common.world")

---@class ServerWorld : World
---@field _replicate_peers table<integer, ServerGameObject>
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

---@return ServerWorld
function _M.new()
    local self = setmetatable({}, _M)
    self._replicate_peers = {}
    return self
end

---@param peer UDPPeer
---@param go ServerGameObject
function _M:AddPeer(peer, go)
    self._replicate_peers[peer:GetID()] = go
end

--- @brief remove peer from world and scene
---@param peer UDPPeer
function _M:RemovePeer(peer)
    local id = peer:GetID()
    local go = self._replicate_peers[id]
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
    self._replicate_peers[peer:GetID()] = nil

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        scene:RemoveEntity(entity)
    end

    self._replicate_peers[id] = nil
end

return _M
