---@class DIDHelpers
local _M = {}
_M.__index = _M

---@param did DID
---@return boolean
function _M.IsCharacterDID(did)
    return (did & TL_Schema.DID.CharacterDID1) ~= 0
end

---@param did DID
---@return boolean
function _M.IsItemDID(did)
    return (did & TL_Schema.DID.ItemDID1) ~= 0
end

---@param did DID
---@return boolean
function _M.IsFXDID(did)
    return (did & TL_Schema.DID.FXDID1) ~= 0
end

---@param did DID
---@return boolean
function _M.IsSkillDID(did)
    return (did & TL_Schema.DID.SkillDID1) ~= 0
end

return _M
