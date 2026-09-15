---@class CommonDirection
---@field Left integer
---@field Right integer
---@field Down integer
---@field Up integer

---@class Common
---@field Direction CommonDirection
local _M = {}


_M.Direction = {
    Left = 0,
    Right = 1,
    Down = 2,
    Up = 3,
}

return _M
