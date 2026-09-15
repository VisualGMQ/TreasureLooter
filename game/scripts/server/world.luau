local ServerGameObject = require("@server/gameobject")
local World = require("@common/world")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

type ServerWorldData = {
    _replicate_peers: {[number]: ServerGameObject.ServerGameObject},
}

export type ServerWorld = typeof(setmetatable({}::ServerWorldData, _M)) & World.World

function _M.new(): ServerWorld
    local self = {} :: ServerWorldData
    self._replicate_peers = {}
    return setmetatable(self, _M)
end

function _M.AddPeer(self: ServerWorld ,peer: UDPPeer, go: ServerGameObject.ServerGameObject)
    self._replicate_peers[peer:GetID()] = go
end

--- @brief remove peer from world and scene
function _M.RemovePeer(self: ServerWorld, peer: UDPPeer)
    local id= peer:GetID()
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