---@class PlayerStates
---@field Root integer
---@field Ordinary integer
---@field FreeHand integer
---@field Carrying integer
---@field PickingUp integer
---@field PuttingDown integer
---@field Frozen integer
---@field Dead integer

local States = {
    Root = 0,
    Ordinary = 1,
    FreeHand = 2,
    Carrying = 3,
    PickingUp = 4,
    PuttingDown = 5,
    Frozen = 6,
    Dead = 7,
}

return States
