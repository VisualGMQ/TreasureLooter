local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")
local LuaContext = require("@game/context")

local Item = {}
Item.__index = Item

export type Item = {
    m_type : TL.ItemType,
    m_behavior: TL.ItemBehaviorFlags,
    m_item_uuid : TL.UUID, -- detemine whether two item are the same type, so they can stack

    -- mine relate
    m_mineral_fragment: TL.ItemDefinitionHandle,
    m_dig_time: number,
}

function Item.new(definition: TL.ItemDefinitionHandle): Item?
    if not definition:IsValid() then
        return nil
    end

    local self = {} :: Item

    self.m_type = definition.m_type
    self.m_behavior = definition.m_behavior
    self.m_item_uuid = definition:GetUUID()
    self.m_dig_time = definition.m_dig_time
    self.m_mineral_fragment = definition.m_mineral_fragment

    return setmetatable(self, Item)
end

function Item.IsSameType(self: Item, other: Item): boolean
    if not self.m_item_uuid:IsValid() then
        return false
    end
    return self.m_item_uuid == other.m_item_uuid
end


local ItemBehavior = {Item = Item}
ItemBehavior.__index = ItemBehavior
setmetatable(ItemBehavior, { __index = ScriptBehavior })

type ItemBehaviorData = {
    m_transform : TL.Transform,
    m_relationship : TL.Relationship,
    m_pickup_trigger : TL.Trigger,

    m_item: Item,
}

export type ItemBehavior = typeof(setmetatable({} :: ItemBehaviorData, ItemBehavior)) & ScriptBehavior.ScriptBehavior

function ItemBehavior.new(entity: TL.Entity): ItemBehavior?
    local self = ScriptBehavior.new(entity)

    return setmetatable(self, ItemBehavior)
end

function ItemBehavior.OnInit(self: ItemBehavior)
    local ctx: TL.GameContext = TL.GetContext()

    local entity = self:GetEntity()
    self.m_transform = ctx:GetTransformManager():Get(entity)
    self.m_relationship = ctx:GetRelationshipManager():Get(entity)
end

function ItemBehavior._init(self: ItemBehavior, item: Item?)
    if not item then
        TL.Log("definition is null")
        self.m_behavior = TL.ItemBehaviorFlags()
        self.m_type = TL.ItemType.Unknown
        return
    end

    self.m_item = item
end

function ItemBehavior.PickupTo(self: ItemBehavior, owner_entity: TL.Entity, stack_index: number)
    if owner_entity == TL.null_entity then
        return
    end

    local ctx: TL.GameContext = TL.GetContext()

    local entity = self:GetEntity()
    local relationship = ctx:GetRelationshipManager():Get(entity)
    relationship:RemoveFromParent()

    local owner_relationship = ctx:GetRelationshipManager():Get(owner_entity)
    owner_relationship:AddChild(entity)

    self.m_transform.m_position.x = 0
    self.m_transform.m_position.y = -10 - stack_index * 5
    ctx:GetStaticCollisionManager():Disable(entity)
end

function ItemBehavior.Putdown(self: ItemBehavior, putdown_position: TL.Vec2)
    local ctx: TL.GameContext = TL.GetContext()

    local entity = self:GetEntity()
    local relationship = ctx:GetRelationshipManager():Get(entity)
    relationship:RemoveFromParent()

    local ground_relationship = ctx:GetRelationshipManager():Get(LuaContext.m_player_layer_entity)
    ground_relationship:AddChild(entity)

    self.m_transform.m_position = putdown_position
    ctx:GetStaticCollisionManager():Enable(entity)
end

return ItemBehavior
