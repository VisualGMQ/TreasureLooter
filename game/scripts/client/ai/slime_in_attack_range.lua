local SlimeUtil = require("client.ai.slime_util")

--- Condition: a player is within contact (attack) range.
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@return table
function _M.new(entity)
    return setmetatable({ _entity = entity }, _M)
end

function _M:OnInit()
end

function _M:OnQuit()
end

---@param entity LogicEntity
---@param blackboard table
---@return BehaviorTreeStatus
function _M:OnUpdate(entity, blackboard)
    local ctx = TL_Client.GetContext()
    if SlimeUtil.FindNearestPlayer(ctx, entity, SlimeUtil.k_attack_range) then
        return TL_Schema.BehaviorTreeStatus.Success
    end
    return TL_Schema.BehaviorTreeStatus.Failure
end

return _M
