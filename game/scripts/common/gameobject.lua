local HpComponent = require("common.components.hp")
local AttackComponent = require("common.components.attack")
local WeaponComponent = require("common.components.weapon")
local ItemComponent = require("common.components.item")
local RaiseUpComponent = require("common.components.raise_up")

---@class GameObject
---@field m_transform Transform
---@field m_hp_component HpComponent
---@field m_attack_component AttackComponent
---@field m_weapon_copmonent WeaponComponent
---@field m_item_component ItemComponent
---@field m_raise_up_component RaiseUpComponent
---@field _entity Entity
---@field _did DID
local _M = {}
_M.__index = _M

---@param entity Entity
---@param did DID
---@return GameObject
function _M.new(entity, did)
    local self = setmetatable({}, _M)
    local ctx = TL_Common.GetContext()
    self._entity = entity
    self._did = did
    self.m_transform = ctx:GetTransformManager():Get(entity)
    return self
end

---@return Entity
function _M:GetEntity()
    return self._entity
end

---@return DID
function _M:GetDID()
    return self._did
end

function _M:Destroy()
    local ctx = TL_Common.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    scene:RemoveEntity(self:GetEntity())
end

return _M
