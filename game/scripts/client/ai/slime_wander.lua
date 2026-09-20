local SlimeUtil = require("client.ai.slime_util")

--- Action: wander. Pick a new random direction every 3 seconds and walk in it.
local _M = {}
_M.__index = _M

local k_wander_interval = 3.0

---@param entity LogicEntity
---@return table
function _M.new(entity)
    return setmetatable({
        _entity = entity,
        _time_left = 0,
        _dir = TL_Common.Vec2.ZERO,
    }, _M)
end

function _M:OnInit()
    -- Keep the wander timer/direction across re-entries: this leaf is ticked
    -- each frame and returns Success so the root Selector re-checks the
    -- higher priority branches instead of staying committed to Wander.
end

function _M:OnQuit()
end

---@param entity LogicEntity
---@param blackboard table
---@return BehaviorTreeStatus
function _M:OnUpdate(entity, blackboard)
    local ctx = TL_Client.GetContext()
    local elapse_time = ctx:GetTime():GetElapseTime()

    self._time_left = self._time_left - elapse_time
    if self._time_left <= 0 then
        self._time_left = k_wander_interval
        local angle = math.random() * math.pi * 2
        self._dir = TL_Common.Vec2(math.cos(angle), math.sin(angle))
    end

    SlimeUtil.MoveInDirection(ctx, entity, self._dir, elapse_time)
    return TL_Schema.BehaviorTreeStatus.Success
end

return _M
