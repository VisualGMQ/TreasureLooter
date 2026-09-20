--- Shared helpers for the slime behavior tree leaves.
local _M = {}

--- Distance within which the slime is considered touching the player.
_M.k_attack_range = 16

---@param ctx any
---@param entity LogicEntity
---@return any
function _M.GetGameObject(ctx, entity)
    local behavior = ctx:GetScriptManager():Get(entity)
    return behavior and behavior.m_gameobject
end

--- Players are the only replicated entities, so their net id is non-zero.
---@param go any
---@return boolean
function _M.IsPlayer(go)
    return go ~= nil and go:GetNetID() ~= 0
end

--- Nearest player currently touching the owner's detect trigger.
---@param ctx any
---@param entity LogicEntity
---@param max_range number|nil
---@return any, number
function _M.FindNearestPlayer(ctx, entity, max_range)
    local trigger = ctx:GetTriggerComponentManager():Get(entity)
    local transform = ctx:GetTransformManager():Get(entity)
    if not trigger or not transform then
        return nil, math.huge
    end

    local owner_pos = transform:GetGlobalPosition()
    local max_sq = max_range and (max_range * max_range) or math.huge
    local nearest = nil
    local nearest_dist = math.huge

    for _, shape in ipairs(trigger:GetTouchingShapes()) do
        local ok, target_entity = pcall(function() return shape:GetOwner() end)
        if ok and target_entity ~= TL_Common.null_entity then
            local go = _M.GetGameObject(ctx, target_entity)
            if _M.IsPlayer(go) then
                local diff = go.m_transform:GetGlobalPosition() - owner_pos
                local dist = diff:LengthSquared()
                if dist <= max_sq and dist < nearest_dist then
                    nearest_dist = dist
                    nearest = go
                end
            end
        end
    end

    return nearest, nearest_dist
end

--- Move one frame in `dir` (normalized internally).
---@param ctx any
---@param entity LogicEntity
---@param dir Vec2
---@param elapse_time TimeType
function _M.MoveInDirection(ctx, entity, dir, elapse_time)
    local go = _M.GetGameObject(ctx, entity)
    local move = go and go.m_move_component
    if not move then
        return
    end

    if dir:LengthSquared() > 0 then
        dir = dir:Normalize()
    end
    move:SetDir(dir)
    move:SetMoveDisp(move:GetVelocity() * elapse_time)
    move:Update(elapse_time)
end

--- Move one frame towards a world position.
---@param ctx any
---@param entity LogicEntity
---@param target_pos Vec2
---@param elapse_time TimeType
function _M.MoveToward(ctx, entity, target_pos, elapse_time)
    local go = _M.GetGameObject(ctx, entity)
    if not go then
        return
    end
    _M.MoveInDirection(ctx, entity,
                       target_pos - go.m_transform:GetGlobalPosition(),
                       elapse_time)
end

--- Stand still and stop the walk animation.
---@param ctx any
---@param entity LogicEntity
---@param elapse_time TimeType
function _M.StopMove(ctx, entity, elapse_time)
    _M.MoveInDirection(ctx, entity, TL_Common.Vec2.ZERO, elapse_time)
end

--- The player shape the slime's own CCT touched on its last move, if any.
---@param ctx any
---@param entity LogicEntity
---@return any
function _M.FindTouchedPlayer(ctx, entity)
    local cct = ctx:GetCCTManager():Get(entity)
    if not cct then
        return nil
    end
    for i = 0, cct:GetTouchedShapeCount() - 1 do
        local shape = cct:GetTouchedShapeAt(i)
        if shape then
            local go = _M.GetGameObject(ctx, shape:GetOwner())
            if _M.IsPlayer(go) then
                return go
            end
        end
    end
    return nil
end

return _M
