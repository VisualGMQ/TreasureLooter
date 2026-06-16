local DID = require("@common/did")

local _M = {}
_M.__index = _M

type ObjectDefinitionTableData = {
    m_characters: {[DID]:CharacterDefinitionHandle},
    m_items: {[DID]:ItemDefinitionHandle},
}
export type ObjectDefinitionTable = typeof(_M) & ObjectDefinitionTableData

function _M.new(definition_table:ObjectDefinitionTableHandle)
    local self = {} :: ObjectDefinitionTableData
    self.m_characters = {}
    self.m_items = {}

    local ctx = TL_Common.GetContext()
    local character_def_mgr = ctx:GetAssetsManager():GetCharacterDefinitionManager()
    local item_def_mgr = ctx:GetAssetsManager():GetItemDefinitionManager()
    for did, path in pairs(definition_table.m_objects) do
        if DID.IsCharacterDID(did) then
            self.m_characters[did] = character_def_mgr:Load(path)
        elseif DID.IsItemDID(did) then
            self.m_items[did] = item_def_mgr:Load(path)
        end
    end

    return setmetatable(self, _M)
end

function _M.GetCharacter(self:ObjectDefinitionTable, did:DID):CharacterDefinitionHandle
    local definition = self.m_characters[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no character asset by DID ", did)
    return TL_Schema.CharacterDefinitionHandle()
end

function _M.GetItem(self:ObjectDefinitionTable, did:DID):ItemDefinitionHandle
    local definition = self.m_items[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no item asset by DID ", did)
    return TL_Schema.ItemDefinitionHandle()
end

function _M.Get(self:ObjectDefinitionTable, did:DID): CharacterDefinitionHandle|ItemDefinitionHandle|nil
    if DID.IsCharacterDID(did) then
        return self:GetCharacter(did)
    elseif DID.IsItemDID(did) then
        return self:GetItem(did)
    else
        return nil
    end
end


return _M