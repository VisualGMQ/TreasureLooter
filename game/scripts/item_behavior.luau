local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")

local ItemBehavior = {}
ItemBehavior.__index = ItemBehavior
setmetatable(ItemBehavior, { __index = ScriptBehavior })

export type Item = {
    m_type : TL.ItemType,
    m_behavior: TL.ItemBehaviorFlags,
}

type ItemBehaviorData = {
    m_transform : TL.Transform,
    m_relationship : TL.Relationship,
    m_pickup_trigger : TL.Trigger,
    m_ground_entity: TL.Entity,

    m_item: Item,
}

export type ItemBehavior = typeof(setmetatable({} :: ItemBehaviorData, ItemBehavior)) & ScriptBehavior.ScriptBehavior

function ItemBehavior.createItem(definition: TL.ItemDefinitionHandle): Item?
    if not definition:IsValid() then
        return nil
    end

    local self = {} :: Item

    self.m_type = definition.m_type
    self.m_behavior = definition.m_behavior

    return self
end

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

function ItemBehavior._init(self: ItemBehavior, item: Item?, ground_entity: TL.Entity)
    if not item then
        TL.Log("definition is null")
        self.m_behavior = TL.ItemBehaviorFlags()
        self.m_type = TL.ItemType.Unknown
        self.m_ground_entity = TL.null_entity
        return
    end

    self.m_item = item
    self.m_ground_entity = ground_entity
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

    ctx:GetTriggerComponentManager():Disable(self:GetEntity())
end

function ItemBehavior.Putdown(self: ItemBehavior, putdown_position: TL.Vec2)
    local ctx: TL.GameContext = TL.GetContext()

    local entity = self:GetEntity()
    local relationship = ctx:GetRelationshipManager():Get(entity)
    relationship:RemoveFromParent()

    local ground_relationship = ctx:GetRelationshipManager():Get(self.m_ground_entity)
    ground_relationship:AddChild(entity)

    self.m_transform.m_position = putdown_position

    ctx:GetTriggerComponentManager():Enable(self:GetEntity())
end

return ItemBehavior
