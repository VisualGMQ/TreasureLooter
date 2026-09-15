local SpawnedComponent = require("common.components.spawned")

---@class ClientSpawnedComponent : SpawnedComponent
---@field _spawn_anim_indices integer[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = SpawnedComponent })

---@param gameobject any
---@param mine_gameobject any
---@param controller_entity Entity
---@param target_x number
---@param spawn_anim_indices integer[]
---@return ClientSpawnedComponent
function _M.new(gameobject, mine_gameobject, controller_entity, target_x, spawn_anim_indices)
    local self = SpawnedComponent.new(gameobject, mine_gameobject, controller_entity, target_x)
    ---@cast self ClientSpawnedComponent
    self._spawn_anim_indices = spawn_anim_indices
    return setmetatable(self, _M)
end

function _M:Stop()
    local client_ctx = TL_Client.GetContext()
    local entity = self._gameobject:GetEntity()
    local anim_mgr = client_ctx:GetAnimationPlayerManager()
    if anim_mgr:Has(entity) then
        for _, idx in ipairs(self._spawn_anim_indices) do
            local anim = anim_mgr:Get(entity, idx)
            if anim then
                anim:Stop()
            end
        end
    end
    SpawnedComponent.Stop(self)
end

return _M
