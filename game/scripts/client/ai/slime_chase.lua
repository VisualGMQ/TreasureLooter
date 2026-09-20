local SlimeUtil = require("client.ai.slime_util")

--- Action: chase the nearest player. Returns Success on contact, Running while
--- still approaching, Failure when there is no target.
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
    local target = SlimeUtil.FindNearestPlayer(ctx, entity)
    if not target then
        return TL_Schema.BehaviorTreeStatus.Failure
    end

    SlimeUtil.MoveToward(ctx, entity, target.m_transform:GetGlobalPosition(),
                         ctx:GetTime():GetElapseTime())

    -- Contact is detected from the slime's own CCT touch shapes.
    if SlimeUtil.FindTouchedPlayer(ctx, entity) then
        return TL_Schema.BehaviorTreeStatus.Success
    end
    return TL_Schema.BehaviorTreeStatus.Running
end

return _M
