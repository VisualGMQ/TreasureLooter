---@class ScriptBehavior
---@field _entity Entity
local ScriptBehavior = {}
ScriptBehavior.__index = ScriptBehavior

---@param entity Entity
---@return ScriptBehavior
function ScriptBehavior.new(entity)
    local self = setmetatable({}, ScriptBehavior)
    self._entity = entity
    return self
end

---@return Entity
function ScriptBehavior:GetEntity()
    return self._entity
end

function ScriptBehavior:OnInit()
end

function ScriptBehavior:OnQuit()
end

---@param elapse_time TimeType
function ScriptBehavior:OnUpdate(elapse_time)
end

function ScriptBehavior:OnRender()
end

return ScriptBehavior
