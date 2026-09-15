local DID = require("common.did")

---@class ObjectDefinitionTable
---@field m_characters table<DID, CharacterDefinitionHandle>
---@field m_items table<DID, ItemDefinitionHandle>
---@field m_fxs table<DID, FXDefinitionHandle>
---@field m_skills table<DID, SkillDefinitionHandle>
local _M = {}
_M.__index = _M

---@param definition_table ObjectDefinitionTableHandle
---@return ObjectDefinitionTable
function _M.new(definition_table)
    local self = setmetatable({}, _M)
    self.m_characters = {}
    self.m_items = {}
    self.m_fxs = {}
    self.m_skills = {}

    local ctx = TL_Common.GetContext()
    local character_def_mgr = ctx:GetAssetsManager():GetCharacterDefinitionManager()
    local item_def_mgr = ctx:GetAssetsManager():GetItemDefinitionManager()
    local fx_def_mgr = ctx:GetAssetsManager():GetFXDefinitionManager()
    local skill_def_mgr = ctx:GetAssetsManager():GetSkillDefinitionManager()
    for did, path in pairs(definition_table.m_objects) do
        if DID.IsCharacterDID(did) then
            self.m_characters[did] = character_def_mgr:Load(path)
        elseif DID.IsItemDID(did) then
            self.m_items[did] = item_def_mgr:Load(path)
        elseif DID.IsFXDID(did) then
            self.m_fxs[did] = fx_def_mgr:Load(path)
        elseif DID.IsSkillDID(did) then
            self.m_skills[did] = skill_def_mgr:Load(path)
        end
    end

    return self
end

---@param did DID
---@return CharacterDefinitionHandle
function _M:GetCharacter(did)
    local definition = self.m_characters[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no character asset by DID ", did)
    return TL_Schema.CharacterDefinitionHandle()
end

---@param did DID
---@return ItemDefinitionHandle
function _M:GetItem(did)
    local definition = self.m_items[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no item asset by DID ", did)
    return TL_Schema.ItemDefinitionHandle()
end

---@param did DID
---@return FXDefinitionHandle
function _M:GetFX(did)
    local definition = self.m_fxs[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no fx asset by DID ", did)
    return TL_Schema.FXDefinitionHandle()
end

---@param did DID
---@return SkillDefinitionHandle
function _M:GetSkill(did)
    local definition = self.m_skills[did]
    if definition then
        return definition
    end
    TL_Common.GetContext():Log("no skill asset by DID ", did)
    return TL_Schema.SkillDefinitionHandle()
end

---@param did DID
---@return CharacterDefinitionHandle|ItemDefinitionHandle|FXDefinitionHandle|SkillDefinitionHandle|nil
function _M:Get(did)
    if DID.IsCharacterDID(did) then
        return self:GetCharacter(did)
    elseif DID.IsItemDID(did) then
        return self:GetItem(did)
    elseif DID.IsFXDID(did) then
        return self:GetFX(did)
    elseif DID.IsSkillDID(did) then
        return self:GetSkill(did)
    else
        return nil
    end
end

return _M
