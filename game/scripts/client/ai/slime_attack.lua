local SlimeUtil = require("client.ai.slime_util")

--- Action: the slime has reached the player. Stop, damage the player that its
--- CCT is touching (falling back to the nearest player in attack range), and
--- let the parent Wait node keep it in place for a second.
local _M = {}
_M.__index = _M

local k_damage = 1

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
    local elapse_time = ctx:GetTime():GetElapseTime()

    -- Stay in place (and stop the walk animation).
    SlimeUtil.StopMove(ctx, entity, elapse_time)

    local target = SlimeUtil.FindTouchedPlayer(ctx, entity)
    if not target then
        target = SlimeUtil.FindNearestPlayer(ctx, entity,
                                             SlimeUtil.k_attack_range)
    end

    if target and target.m_hp_component then
        target.m_hp_component:Hurt(k_damage)
    end

    return TL_Schema.BehaviorTreeStatus.Success
end

return _M
