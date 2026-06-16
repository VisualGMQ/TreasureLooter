local Component = require("@common/components/component")

local _M = {}
_M.__index = {}
setmetatable(_M, {__index = Component})

type WeaponComponentData = {
    _weapon_script:any, -- is Client/ServerGameObjectBehavior
}

export type WeaponComponent = typeof(_M) & WeaponComponentData & Component.Component

function _M.new(gameobject):WeaponComponent
    local self = Component.new(gameobject) :: WeaponComponent
    return setmetatable(self, _M)
end

function _M.GetScript(self:WeaponComponent)
    return self._weapon_script
end


return _M