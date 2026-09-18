---@class GameObject
---@field m_transform Transform
---@field m_hp_component HpComponent
---@field m_attack_component AttackComponent
---@field m_weapon_copmonent WeaponComponent
---@field m_item_component ItemComponent
---@field m_raise_up_component RaiseUpComponent
---@field private _entity LogicEntity
---@field private _net_id number
---@field private _did DID
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@param did DID
---@param net_id number?
---@return GameObject
function _M.new(entity, did, net_id)
    local self = setmetatable({}, _M)
    local ctx = TL_Common.GetContext()
    self._entity = entity
    self._did = did
    self._net_id = net_id or 0
    local transform = ctx:GetTransformManager():Get(entity)
    ---@cast transform Transform
    self.m_transform = transform
    return self
end

function _M:GetNetID()
    return self._net_id
end

---@return LogicEntity
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
    if scene then
        scene:RemoveEntity(self:GetEntity())
    end
end

return _M
