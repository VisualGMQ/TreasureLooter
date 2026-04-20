local TL = require("@game/tl_types")

local Item = {}
Item.__index = Item

type ItemData = {
    m_type : TL.ItemType,
    m_behavior: TL.ItemBehaviorFlag,

    m_transform: TL.Transform?,
}

export type Item = typeof(setmetatable({} :: ItemData, Item))

function Item.new(entity: TL.Entity, definition: TL.ItemDefinitionHandle): Item?
    local self = {} :: ItemData

    if not definition:IsValid() then
        TL.Log("definition is null")
        return nil
    end

    local ctx: TL.GameContext = TL.GetContext()

    self.m_type = definition.m_type
    self.m_behavior = definition.m_behavior

    self.m_transform = ctx:GetTransformManager():Get(entity)

    return setmetatable(self, Item)
end

return Item

