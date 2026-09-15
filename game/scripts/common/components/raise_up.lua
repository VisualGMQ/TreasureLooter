local Copmonent = require("common.components.component")
local World = require("common.world")

---@class RaiseUpComponent : Component
---@field _did DID
---@field _items any[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Copmonent })

---@param gameobject any
---@return RaiseUpComponent
function _M.new(gameobject)
    local self = Copmonent.new(gameobject)
    ---@cast self RaiseUpComponent
    self._did = TL_Schema.DID.Invalid
    self._items = {}
    return setmetatable(self, _M)
end

---@return boolean
function _M:IsHolding()
    return #self._items > 0
end

---@return integer
function _M:GetHoldingItemCount()
    return #self._items
end

---@param gameobject any
---@return boolean
function _M:TryRaiseUp(gameobject)
    local item_component = gameobject.m_item_component
    if not item_component then
        return false
    end

    if not item_component:GetBehavior():Has(TL_Schema.ItemBehavior.Pickupable) then
        return false
    end

    local did = gameobject:GetDID()
    if #self._items == 0 then
        self._did = did
    elseif did ~= self._did then
        return false
    end

    local ctx = TL_Common.GetContext()
    local rel_mgr = ctx:GetRelationshipManager()

    local item_entity = gameobject:GetEntity()
    local item_relationship = rel_mgr:Get(item_entity)
    if not item_relationship then
        return false
    end

    local owner_relationship = rel_mgr:Get(self._gameobject:GetEntity())
    if not owner_relationship then
        return false
    end

    item_relationship:RemoveFromParent()
    owner_relationship:AddChild(item_entity)
    gameobject.m_transform.m_position = TL_Common.Vec2(0, 0)

    local static_collision_mgr = ctx:GetStaticCollisionManager()
    if static_collision_mgr:Has(item_entity) then
        static_collision_mgr:Disable(item_entity)
    end

    table.insert(self._items, gameobject)
    return true
end

---@param position Vec2
---@return any|nil
function _M:PutDown(position)
    local count = #self._items
    if count == 0 then
        return nil
    end

    local gameobject = self._items[count]

    local ctx = TL_Common.GetContext()
    local rel_mgr = ctx:GetRelationshipManager()
    local item_entity = gameobject:GetEntity()

    local item_relationship = rel_mgr:Get(item_entity)
    local land_relationship = rel_mgr:Get(World.GetInst().m_land_entity)
    if item_relationship and land_relationship then
        item_relationship:RemoveFromParent()
        land_relationship:AddChild(item_entity)

        local land_transform = ctx:GetTransformManager():Get(World.GetInst().m_land_entity)
        if land_transform then
            gameobject.m_transform.m_position = position - land_transform:GetGlobalPosition()
        else
            gameobject.m_transform.m_position = position
        end
    end

    -- back on the ground, make it interactable again
    local static_collision_mgr = ctx:GetStaticCollisionManager()
    if static_collision_mgr:Has(item_entity) then
        static_collision_mgr:Enable(item_entity)
    end

    table.remove(self._items, count)
    if #self._items == 0 then
        self._did = TL_Schema.DID.Invalid
    end

    return gameobject
end

return _M
