local WeaponComponent = require("@common/components/weapon")
local _M = {}
_M.__index = _M
setmetatable(_M, {__index = WeaponComponent})

type ClientWeaponComponentData = { }

export type ClientWeaponComponent = typeof(_M) & WeaponComponent.WeaponComponent

export type ClientWeaponComponentDefinition = {
    m_weapon_entity:Entity,
}

function _M.new(gameobject, definition:ClientWeaponComponentDefinition):ClientWeaponComponent
    local self = WeaponComponent.new(gameobject) :: ClientWeaponComponent
    local ctx = TL_Client.GetContext()
    local script = ctx:GetScriptManager():Get(definition.m_weapon_entity)
    if script then
        self._weapon_script = script
    else
        ctx:Log("weapon don't has script")
    end
    return setmetatable(self, _M)
end


return _M