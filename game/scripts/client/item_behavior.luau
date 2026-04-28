local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")
local TL_Schema = require("@type_hints/tl_schema_types")


local ScriptBehavior = require("@client/script_behavior")
local LuaContext = require("@client/context")

local Item = {}
Item.__index = Item

export type Item = {
    m_type : TL_Schema.ItemType,
    m_behavior: TL_Schema.ItemBehaviorFlags,
    m_item_uuid : TL_Common.UUID, -- detemine whether two item are the same type, so they can stack

    -- mine relate
    m_mineral_fragment: TL_Schema.ItemDefinitionHandle,
    m_dig_time: number,
}

function Item.new(definition: TL_Schema.ItemDefinitionHandle): Item?
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
    m_transform : TL_Common.Transform,
    m_relationship : TL_Common.Relationship,
    m_pickup_trigger : TL_Common.Trigger,

    m_item: Item,
}

export type ItemBehavior = typeof(setmetatable({} :: ItemBehaviorData, ItemBehavior)) & ScriptBehavior.ScriptBehavior

function ItemBehavior.new(entity: TL_Common.Entity): ItemBehavior?
    local self = ScriptBehavior.new(entity)

    return setmetatable(self, ItemBehavior)
end

function ItemBehavior.OnInit(self: ItemBehavior)
    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local entity = self:GetEntity()
    self.m_transform = ctx:GetTransformManager():Get(entity)
    self.m_relationship = ctx:GetRelationshipManager():Get(entity)
end

function ItemBehavior._init(self: ItemBehavior, item: Item?)
    if not item then
        TL_Common.Log("definition is null")
        self.m_behavior = TL_Schema.ItemBehaviorFlags()
        self.m_type = TL_Schema.ItemType.Unknown
        return
    end

    self.m_item = item
end

function ItemBehavior.PickupTo(self: ItemBehavior, owner_entity: TL_Common.Entity, stack_index: number)
    if owner_entity == TL_Common.null_entity then
        return
    end

    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local entity = self:GetEntity()
    local relationship = ctx:GetRelationshipManager():Get(entity)
    relationship:RemoveFromParent()

    local owner_relationship = ctx:GetRelationshipManager():Get(owner_entity)
    owner_relationship:AddChild(entity)

    self.m_transform.m_position.x = 0
    self.m_transform.m_position.y = -10 - stack_index * 5
    ctx:GetStaticCollisionManager():Disable(entity)
end

function ItemBehavior.Putdown(self: ItemBehavior, putdown_position: TL_Common.Vec2)
    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local entity = self:GetEntity()
    local relationship = ctx:GetRelationshipManager():Get(entity)
    relationship:RemoveFromParent()

    local ground_relationship = ctx:GetRelationshipManager():Get(LuaContext.m_player_layer_entity)
    ground_relationship:AddChild(entity)

    self.m_transform.m_position = putdown_position
    ctx:GetStaticCollisionManager():Enable(entity)
end

return ItemBehavior


