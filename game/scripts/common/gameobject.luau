local HpComponent = require("@common/components/hp")
local AttackComponent = require("@common/components/attack")
local WeaponComponent = require("@common/components/weapon")
local ItemComponent = require("@common/components/item")
local RaiseUpComponent = require("@common/components/raise_up")
local _M = {}
_M.__index = _M

type GameObjectData = {
    m_transform:Transform,
    m_hp_component:HpComponent.HpComponent,
    m_attack_component:AttackComponent.AttackComponent,
    m_weapon_copmonent:WeaponComponent.WeaponComponent,
    m_item_component:ItemComponent.ItemComponent,
    m_raise_up_component:RaiseUpComponent.RaiseUpComponent,

    -- private fields
    _entity:Entity,
    _did:DID,
}
export type GameObject = typeof(_M) & GameObjectData

function _M.new(entity:Entity, did:DID):GameObject
    local self = {}::GameObjectData
    local ctx = TL_Common.GetContext()
    self._entity = entity
    self._did = did
    self.m_transform = ctx:GetTransformManager():Get(entity)
    return setmetatable(self, _M)
end

function _M.GetEntity(self:GameObject):Entity
    return self._entity
end

function _M.GetDID(self:GameObject):DID
    return self._did
end

function _M.Destroy(self:GameObject)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    scene:RemoveEntity(self:GetEntity())
end

return _M