local _M = {}
_M.__index = _M

function _M.IsCharacterDID(did:DID):boolean
    return bit32.band(did, TL_Schema.DID.CharacterDID1) ~= 0
end

function _M.IsItemDID(did:DID):boolean
    return bit32.band(did, TL_Schema.DID.ItemDID1) ~= 0
end


return _M