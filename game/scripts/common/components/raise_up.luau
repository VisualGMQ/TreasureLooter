local Copmonent = require("@common/components/component")
local World = require("@common/world")

local _M = {}
_M.__index = _M
setmetatable(_M, Copmonent)

type RaiseUpComponentData = {
    _did:DID,
    _items:{any},
}

export type RaiseUpComponent = typeof(_M) & RaiseUpComponentData & Copmonent.Component

function _M.new(gameobject:any):RaiseUpComponent
    local self = Copmonent.new(gameobject) :: RaiseUpComponent
    self._did = TL_Schema.DID.Invalid
    self._items = {}
    return setmetatable(self, _M)
end

function _M.IsHolding(self:RaiseUpComponent):boolean
    return #self._items > 0
end

-- visual hooks, overridden by the client component (lift/restore the sprite via anchor)
function _M.onItemRaise(self:RaiseUpComponent, gameobject:any, index:number)
end

function _M.onItemPutDown(self:RaiseUpComponent, gameobject:any)
end

function _M.TryRaiseUp(self:RaiseUpComponent, gameobject:any):boolean
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

    -- keep the item as a child of the owner so its global position always follows the owner.
    -- y-sorting uses the global y (see sprite.cpp), so they occlude together; the visual
    -- head/stack offset is done by onItemRaise (sprite anchor), which doesn't move the entity.
    item_relationship:RemoveFromParent()
    owner_relationship:AddChild(item_entity)
    gameobject.m_transform.m_position = TL_Common.Vec2(0, 0)
    self:onItemRaise(gameobject, #self._items)

    -- carried items shouldn't be detected as ground interactables anymore
    local static_collision_mgr = ctx:GetStaticCollisionManager()
    if static_collision_mgr:Has(item_entity) then
        static_collision_mgr:Disable(item_entity)
    end

    table.insert(self._items, gameobject)
    return true
end

function _M.PutDown(self:RaiseUpComponent, position:Vec2):any?
    local count = #self._items
    if count == 0 then
        return nil
    end

    local gameobject = self._items[count]

    local ctx = TL_Common.GetContext()
    local rel_mgr = ctx:GetRelationshipManager()
    local item_entity = gameobject:GetEntity()

    self:onItemPutDown(gameobject)

    local item_relationship = rel_mgr:Get(item_entity)
    local land_relationship = rel_mgr:Get(World.GetInst().m_land_entity)
    if item_relationship and land_relationship then
        item_relationship:RemoveFromParent()
        land_relationship:AddChild(item_entity)

        -- 停止动画，防止后续帧覆盖位置
        local client_ctx = TL_Client.GetContext()
        local multi_anim = client_ctx:GetMultiAnimationPlayerManager():Get(item_entity)
        if multi_anim then
            multi_anim:StopAll()
        end

        -- position 是世界坐标，需转换为 land_entity 下的本地坐标
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
