-- Loads the BuffAppliersTable asset and maps each BuffType to its buff module
-- (the script registered for that buff). Used to instantiate buff objects from
-- a BuffDefinition at runtime.

---@class BuffApplierTable
---@field m_appliers table<integer, any>
local _M = {}
_M.__index = _M

local k_buff_appliers_path = "assets/gpa/buff_appliers.buff_appliers.xml"

---@return BuffApplierTable
function _M.new()
    local self = setmetatable({}, _M)
    self.m_appliers = {}

    local table_data = TL_Schema.LoadAssetBuffAppliersTable(TL_Common.Path(k_buff_appliers_path))
    if table_data and table_data.m_buffs then
        for buff_type, path in pairs(table_data.m_buffs) do
            local modname = path:string()
            local ok, mod = pcall(require, modname)
            if ok and mod then
                self.m_appliers[buff_type] = mod
            else
                TL_Common.GetContext():Log("[buff] failed to require buff applier: ", modname)
            end
        end
    end

    return self
end

--- Creates a buff instance bound to `gameobject` from a BuffDefinition, or nil if
--- no applier is registered for the buff type.
---@param buff_definition BuffDefinition
---@param gameobject any
---@return any
function _M:CreateBuff(buff_definition, gameobject)
    local mod = self.m_appliers[buff_definition.m_type]
    if not mod then
        TL_Common.GetContext():Log("[buff] no applier registered for buff type ", buff_definition.m_type)
        return nil
    end
    return mod.new(buff_definition, gameobject)
end

return _M
